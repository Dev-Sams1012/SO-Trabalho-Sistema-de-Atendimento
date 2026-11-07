#include "recepcao.h"

#include "atendimento/atendimento.h"
#include "escalonador/escalonador.h"
#include "relogio/relogio.h"

#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

// Cria um processo cliente. Retorna o PID ou -1 em caso de erro.
static pid_t criar_cliente(void) {
  char *argv[] = {(char *)CLIENTE_CAMINHO, NULL};
  pid_t pid;

  int erro = posix_spawn(&pid, CLIENTE_CAMINHO, NULL, NULL, argv, environ);
  if (erro != 0) {
    fprintf(stderr, "recepcao: nao foi possivel iniciar %s: %s\n",
            CLIENTE_CAMINHO, strerror(erro));
    return -1;
  }
  return pid;
}

// Le o tempo de atendimento que o cliente gravou. Retorna -1 em caso de erro.
static int ler_demanda(void) {
  FILE *arquivo = fopen(ATENDIMENTO_ARQUIVO_DEMANDA, "r");
  if (arquivo == NULL)
    return -1;

  int tempo;
  int lido = fscanf(arquivo, "%d", &tempo) == 1;
  fclose(arquivo);
  return lido ? tempo : -1;
}

// Mata e recolhe um cliente que nao chegou a entrar na fila.
static void descartar_cliente(pid_t pid) {
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

static void falhar(Atendimento *atendimento, const char *mensagem) {
  fprintf(stderr, "recepcao: %s\n", mensagem);
  atendimento->erro = true;
  atendimento->parar = 1;
}

void recepcao_thread(void *argumento) {
  Atendimento *atendimento = argumento;

  while (!atendimento->parar &&
         (atendimento->n == 0 || atendimento->criados < atendimento->n)) {
    // Fila com 100 esperando: pausa a criacao ate abrir espaco.
    if (fila_cheia(&atendimento->fila)) {
      escalonador_ceder();
      continue;
    }

    pid_t pid = criar_cliente();
    if (pid < 0) {
      falhar(atendimento, "falha ao criar cliente");
      break;
    }
    long long hora_chegada = relogio_atual_ms();

    // O cliente grava a demanda e se para sozinho; esperamos sem travar as
    // outras threads.
    int estado;
    if (escalonador_esperar_processo(pid, &estado, WUNTRACED) != pid ||
        !WIFSTOPPED(estado)) {
      // Ctrl+C tambem atinge o cliente recem-criado: se ja estamos parando,
      // isso nao eh uma falha.
      if (!atendimento->parar)
        falhar(atendimento, "cliente terminou antes de parar");
      break;
    }

    int tempo = ler_demanda();
    if (tempo < 0) {
      descartar_cliente(pid);
      falhar(atendimento, "nao foi possivel ler a demanda do cliente");
      break;
    }

    Cliente cliente = {
        .pid = pid,
        .hora_chegada = hora_chegada,
        .prioridade = (rand() % 2 == 0) ? PRIORIDADE_ALTA : PRIORIDADE_BAIXA,
        .tempo_atendimento = tempo,
    };
    
    if (fila_inserir(&atendimento->fila, cliente) != 0) {
      descartar_cliente(pid);
      falhar(atendimento, "nao foi possivel inserir cliente na fila");
      break;
    }

    atendimento->criados++;
  }

  atendimento->recepcao_terminou = true;
}
