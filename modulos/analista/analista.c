#include "analista.h"

#include "lng/lng.h"
#include "semaforo/semaforo.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

// Grava o PID deste processo em ANALISTA_ARQUIVO_PID.
static int gravar_pid(void) {
  FILE *arquivo = fopen(ANALISTA_ARQUIVO_PID, "w");
  if (arquivo == NULL) {
    perror("analista: criar " ANALISTA_ARQUIVO_PID);
    return -1;
  }

  int escrita = fprintf(arquivo, "%d\n", (int)getpid());
  int fechamento = fclose(arquivo);

  if (escrita < 0 || fechamento != 0) {
    fprintf(stderr, "analista: erro ao gravar " ANALISTA_ARQUIVO_PID "\n");
    return -1;
  }
  return 0;
}

// Fecha o semaforo (sem_wait), repetindo se um sinal interromper a espera.
static int travar(sem_t *semaforo) {
  while (sem_wait(semaforo) != 0) {
    if (errno != EINTR)
      return -1;
  }
  return 0;
}

int main(void) {
  sem_t *sem_block = semaforo_abrir(SEMAFORO_NOME_BLOCK);
  if (sem_block == SEM_FAILED) {
    perror("analista: abrir " SEMAFORO_NOME_BLOCK);
    return 1;
  }

  if (gravar_pid() != 0)
    return 1;

  pid_t pids[ANALISTA_MAX_POR_VEZ];
    for (;;) {
    raise(SIGSTOP);

    if (travar(sem_block) != 0) {
      perror("analista: sem_wait");
      return 1;
    }
    int quantidade = lng_ler_remover(ANALISTA_MAX_POR_VEZ, pids);
    sem_post(sem_block);

    if (quantidade < 0) {
      fprintf(stderr, "analista: erro ao ler o LNG\n");
      continue;
    }

    for (int i = 0; i < quantidade; i++)
      printf("%d\n", (int)pids[i]);
    fflush(stdout);
  }
}