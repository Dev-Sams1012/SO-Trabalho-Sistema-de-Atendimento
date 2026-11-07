#include "atendimento.h"

#include "analista/analista.h"
#include "escalonador/escalonador.h"
#include "lng/lng.h"
#include "relogio/relogio.h"
#include "semaforo/semaforo.h"

#include <semaphore.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

extern char **environ;

// O tratador de sinal nao recebe argumentos, por isso precisa deste ponteiro.
static Atendimento *atendimento_dos_sinais = NULL;

static void tratar_sinal(int sinal) {
  (void)sinal;
  if (atendimento_dos_sinais != NULL)
    atendimento_dos_sinais->parar = 1;
}

static void apagar_arquivos(void) {
  unlink(ATENDIMENTO_ARQUIVO_DEMANDA);
  unlink(LNG_ARQUIVO);
  unlink(LNG_ARQUIVO_TEMP);
  unlink(ANALISTA_ARQUIVO_PID);
}

// Le o PID que o analista gravou ao nascer. Retorna -1 em caso de erro.
static pid_t ler_pid_analista(void) {
  FILE *arquivo = fopen(ANALISTA_ARQUIVO_PID, "r");
  if (arquivo == NULL)
    return -1;

  int pid;
  int lido = fscanf(arquivo, "%d", &pid) == 1;
  fclose(arquivo);
  return lido ? (pid_t)pid : -1;
}

// Cria o analista e espera ele gravar o PID e dormir (primeira parada).
static int iniciar_analista(Atendimento *atendimento) {
  char *argv[] = {(char *)ANALISTA_CAMINHO, NULL};
  pid_t pid;

  int erro = posix_spawn(&pid, ANALISTA_CAMINHO, NULL, NULL, argv, environ);
  if (erro != 0) {
    fprintf(stderr, "atendimento: nao foi possivel iniciar %s: %s\n",
            ANALISTA_CAMINHO, strerror(erro));
    return -1;
  }
  atendimento->pid_analista = pid; // permite matar o processo se algo falhar

  int estado;
  if (waitpid(pid, &estado, WUNTRACED) != pid || !WIFSTOPPED(estado)) {
    fprintf(stderr, "atendimento: o analista terminou antes de dormir\n");
    atendimento->pid_analista = -1;
    return -1;
  }

  if (ler_pid_analista() != pid) {
    fprintf(stderr, "atendimento: PID do analista em %s nao confere\n",
            ANALISTA_ARQUIVO_PID);
    return -1;
  }
  return 0;
}

static void instalar_tratadores(Atendimento *atendimento) {
  atendimento_dos_sinais = atendimento;

  // Sem SA_RESTART de proposito: o sinal interrompe chamadas bloqueantes.
  struct sigaction acao;
  memset(&acao, 0, sizeof acao);
  acao.sa_handler = tratar_sinal;
  sigemptyset(&acao.sa_mask);
  sigaction(SIGINT, &acao, NULL);
  sigaction(SIGTERM, &acao, NULL);
}

// Mata o que ainda estiver vivo e remove semaforos e arquivos.
static void liberar_recursos(Atendimento *atendimento) {
  // Clientes que sobraram na fila estao parados (SIGSTOP): mata e recolhe.
  while (!fila_vazia(&atendimento->fila)) {
    Cliente *cliente = fila_espiar(&atendimento->fila);
    kill(cliente->pid, SIGKILL);
    waitpid(cliente->pid, NULL, 0);
    fila_remover(&atendimento->fila);
  }

  if (atendimento->pid_analista > 0) {
    kill(atendimento->pid_analista, SIGKILL);
    waitpid(atendimento->pid_analista, NULL, 0);
    atendimento->pid_analista = -1;
  }

  semaforo_destruir(SEMAFORO_NOME_ATEND, atendimento->sem_atend);
  semaforo_destruir(SEMAFORO_NOME_BLOCK, atendimento->sem_block);
  atendimento->sem_atend = SEM_FAILED;
  atendimento->sem_block = SEM_FAILED;

  apagar_arquivos();
}

int atendimento_iniciar(Atendimento *atendimento, int n, int x) {
  memset(atendimento, 0, sizeof *atendimento);
  atendimento->n = n;
  atendimento->x = x;
  atendimento->sem_atend = SEM_FAILED;
  atendimento->sem_block = SEM_FAILED;
  atendimento->pid_analista = -1;

  relogio_iniciar();
  srand((unsigned)time(NULL) ^ (unsigned)getpid());
  escalonador_iniciar();
  fila_iniciar(&atendimento->fila);
  apagar_arquivos(); // sobras de uma execucao anterior

  atendimento->sem_atend = semaforo_criar(SEMAFORO_NOME_ATEND);
  atendimento->sem_block = semaforo_criar(SEMAFORO_NOME_BLOCK);
  if (atendimento->sem_atend == SEM_FAILED ||
      atendimento->sem_block == SEM_FAILED) {
    perror("atendimento: criar semaforos");
    liberar_recursos(atendimento);
    return -1;
  }

  // O analista abre /sem_block, entao os semaforos precisam existir antes.
  if (iniciar_analista(atendimento) != 0) {
    liberar_recursos(atendimento);
    return -1;
  }

  instalar_tratadores(atendimento);
  return 0;
}

void atendimento_acordar_analista(Atendimento *atendimento) {
  if (atendimento->pid_analista <= 0)
    return;

  if (kill(atendimento->pid_analista, SIGCONT) != 0) {
    perror("atendimento: acordar analista");
    atendimento->pid_analista = -1;
    return;
  }

  // SIGCONT nao enfileira: so acordamos de novo depois que ele dormir.
  int estado;
  pid_t resultado = escalonador_esperar_processo(atendimento->pid_analista,
                                                 &estado, WUNTRACED);
  if (resultado < 0 || !WIFSTOPPED(estado)) {
    fprintf(stderr, "atendimento: o analista terminou inesperadamente\n");
    atendimento->pid_analista = -1;
    return;
  }

  atendimento->pendentes_lng -= ANALISTA_MAX_POR_VEZ;
  if (atendimento->pendentes_lng < 0)
    atendimento->pendentes_lng = 0;
}

void atendimento_finalizar(Atendimento *atendimento) {
  // Antes de encerrar, o LNG precisa ser esvaziado.
  while (atendimento->pendentes_lng > 0 && atendimento->pid_analista > 0)
    atendimento_acordar_analista(atendimento);

  long long tempo_total_ms = relogio_atual_ms();

  double taxa = 0.0;
  if (atendimento->atendidos > 0)
    taxa = 100.0 * atendimento->satisfeitos / atendimento->atendidos;

  fflush(stdout); // a saida do analista ja foi escrita; mantem a ordem
  printf("Taxa de satisfação: %.2f%% (%d de %d clientes atendidos)\n", taxa,
         atendimento->satisfeitos, atendimento->atendidos);
  printf("Tempo total de execução: %lld ms\n", tempo_total_ms);
  fflush(stdout);

  liberar_recursos(atendimento);
}
