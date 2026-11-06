#include "analista/analista.h"
#include "lng/lng.h"
#include "semaforo/semaforo.h"

#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

#define ARQUIVO_SAIDA "analista_saida.txt"

#define VERIFICAR(condicao)                                                    \
  do {                                                                         \
    if (!(condicao)) {                                                         \
      fprintf(stderr, "FALHOU %s:%d: %s\n", __FILE__, __LINE__, #condicao);    \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

static void apagar_arquivos(void) {
  unlink(LNG_ARQUIVO);
  unlink(LNG_ARQUIVO_TEMP);
  unlink(ANALISTA_ARQUIVO_PID);
  unlink(ARQUIVO_SAIDA);
}

// PID do analista em execucao (0 se nao ha nenhum).
static pid_t analista_ativo = 0;

// Mata e recolhe o analista. Eh chamada tambem por atexit, para que uma
// verificacao que falhe (exit(1)) nao deixe um processo parado para tras.
static void encerrar_analista(void) {
  if (analista_ativo > 0) {
    kill(analista_ativo, SIGKILL);
    waitpid(analista_ativo, NULL, 0);
    analista_ativo = 0;
  }
}

// Inicia o analista com a saida padrao redirecionada para ARQUIVO_SAIDA.
static pid_t iniciar_analista(void) {
  posix_spawn_file_actions_t acoes;
  posix_spawn_file_actions_init(&acoes);
  posix_spawn_file_actions_addopen(&acoes, STDOUT_FILENO, ARQUIVO_SAIDA,
                                   O_WRONLY | O_CREAT | O_TRUNC, 0644);

  char *argv[] = {(char *)ANALISTA_CAMINHO, NULL};
  pid_t pid;
  int erro = posix_spawn(&pid, ANALISTA_CAMINHO, &acoes, NULL, argv, environ);
  posix_spawn_file_actions_destroy(&acoes);

  VERIFICAR(erro == 0);
  return pid;
}

// Espera o analista PARAR (dormir). O waitpid com WUNTRACED avisa quando um
// filho para por SIGSTOP; cada parada eh avisada uma unica vez.
static void esperar_dormir(pid_t pid) {
  int status;
  VERIFICAR(waitpid(pid, &status, WUNTRACED) == pid);
  VERIFICAR(WIFSTOPPED(status));
}

// Acorda o analista e espera ele terminar o trabalho e dormir de novo.
static void acordar(pid_t pid) {
  VERIFICAR(kill(pid, SIGCONT) == 0);
  esperar_dormir(pid);
}

// Le os numeros que o analista imprimiu ate agora.
static int ler_saida(int *valores, int maximo) {
  FILE *arquivo = fopen(ARQUIVO_SAIDA, "r");
  VERIFICAR(arquivo != NULL);

  int quantidade = 0;
  while (quantidade < maximo &&
         fscanf(arquivo, "%d", &valores[quantidade]) == 1)
    quantidade++;

  fclose(arquivo);
  return quantidade;
}

// Verdadeiro se o semaforo esta aberto (e o deixa aberto como estava).
static bool semaforo_livre(sem_t *semaforo) {
  if (sem_trywait(semaforo) != 0)
    return false;
  sem_post(semaforo);
  return true;
}

int main(void) {
  int saida[64];

  printf("teste_analista\n");
  atexit(encerrar_analista);
  apagar_arquivos();
  semaforo_destruir(SEMAFORO_NOME_BLOCK, SEM_FAILED); // limpa sobras

  // 1. Sem o semaforo /sem_block o analista deve falhar logo, sem deixar
  //    um arquivo de PID apontando para um processo que nao existe.
  {
    pid_t pid = iniciar_analista();
    int status;
    VERIFICAR(waitpid(pid, &status, 0) == pid);
    VERIFICAR(WIFEXITED(status) && WEXITSTATUS(status) == 1);
    VERIFICAR(access(ANALISTA_ARQUIVO_PID, F_OK) != 0);
    printf("  ok: sem semaforo o analista termina com erro e nao grava PID\n");
  }

  sem_t *sem_block = semaforo_criar(SEMAFORO_NOME_BLOCK);
  VERIFICAR(sem_block != SEM_FAILED);

  pid_t analista = iniciar_analista();
  analista_ativo = analista;
  esperar_dormir(analista); // primeira parada: ja gravou o PID e esta dormindo

  // 2. O arquivo de PID contem o PID verdadeiro do analista.
  FILE *arquivo_pid = fopen(ANALISTA_ARQUIVO_PID, "r");
  VERIFICAR(arquivo_pid != NULL);
  int pid_lido = 0;
  VERIFICAR(fscanf(arquivo_pid, "%d", &pid_lido) == 1);
  fclose(arquivo_pid);
  VERIFICAR(pid_lido == (int)analista);
  printf("ok: nasce dormindo e grava o proprio PID em %s\n", ANALISTA_ARQUIVO_PID);

    // 3. Acordado com o LNG vazio: nao imprime nada e volta a dormir.
  acordar(analista);
  VERIFICAR(semaforo_livre(sem_block));
  VERIFICAR(ler_saida(saida, 64) == 0);
  printf("  ok: acordado com o LNG vazio, nao imprime nada\n");

  // 4. Com 25 PIDs no LNG, cada despertar imprime no maximo 10 (10+10+5).
  //    (Podemos gravar sem travar o semaforo porque o analista dorme.)
  for (int i = 0; i < 25; i++)
    VERIFICAR(lng_adicionar(2000 + i) == 0);

  acordar(analista);
  VERIFICAR(semaforo_livre(sem_block)); // o analista soltou o semaforo
  VERIFICAR(ler_saida(saida, 64) == 10);
  for (int i = 0; i < 10; i++)
    VERIFICAR(saida[i] == 2000 + i);

  acordar(analista);
  VERIFICAR(semaforo_livre(sem_block));
  VERIFICAR(ler_saida(saida, 64) == 20);
  for (int i = 0; i < 20; i++)
    VERIFICAR(saida[i] == 2000 + i);

  acordar(analista);
  VERIFICAR(semaforo_livre(sem_block));
  VERIFICAR(ler_saida(saida, 64) == 25);
  for (int i = 0; i < 25; i++)
    VERIFICAR(saida[i] == 2000 + i);
  printf("  ok: 25 PIDs impressos em 3 despertares (10 + 10 + 5), em ordem\n");

    // 5. Os PIDs impressos foram apagados do LNG: mais um despertar nao repete.
  acordar(analista);
  VERIFICAR(ler_saida(saida, 64) == 25);
  pid_t resto[ANALISTA_MAX_POR_VEZ];
  VERIFICAR(lng_ler_remover(ANALISTA_MAX_POR_VEZ, resto) == 0);
  printf("  ok: o que foi impresso saiu do LNG\n");

  // Encerramento: o analista esta parado, e SIGKILL funciona mesmo assim.
  encerrar_analista();
  semaforo_destruir(SEMAFORO_NOME_BLOCK, sem_block);
  apagar_arquivos();

  printf("teste_analista: tudo certo\n");
  return 0;
}

