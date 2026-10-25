#include "escalonador.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ucontext.h>

#define TAMANHO_PILHA (128 * 1024)

typedef enum { THREAD_LIVRE, THREAD_PRONTA, THREAD_FINALIZADA } EstadoThread;

typedef struct {
  ucontext_t contexto;
  EstadoThread estado;
  FuncaoThread funcao;
  void *argumento;
  char *pilha;
} Thread;

static Thread threads[ESCALONADOR_MAX_THREADS];
static ucontext_t contexto_escalonador;
static int thread_atual = -1;
static int threads_ativas = 0;

// Ponto de entrada de toda thread: executa a funcao e marca como finalizada.
// Ao retornar, o uc_link devolve o controle ao escalonador.
static void executar_thread(void) {
  Thread *thread = &threads[thread_atual];
  thread->funcao(thread->argumento);
  thread->estado = THREAD_FINALIZADA;
  threads_ativas--;
}

void escalonador_iniciar(void) {
  for (int i = 0; i < ESCALONADOR_MAX_THREADS; i++) {
    threads[i].estado = THREAD_LIVRE;
    threads[i].pilha = NULL;
  }
  thread_atual = -1;
  threads_ativas = 0;
}

int escalonador_criar_thread(FuncaoThread funcao, void *argumento) {
  for (int i = 0; i < ESCALONADOR_MAX_THREADS; i++) {
    if (threads[i].estado != THREAD_LIVRE)
      continue;

    threads[i].pilha = malloc(TAMANHO_PILHA);
    if (threads[i].pilha == NULL)
      return -1;

    if (getcontext(&threads[i].contexto) != 0) {
      free(threads[i].pilha);
      threads[i].pilha = NULL;
      return -1;
    }
    threads[i].contexto.uc_stack.ss_sp = threads[i].pilha;
    threads[i].contexto.uc_stack.ss_size = TAMANHO_PILHA;
    threads[i].contexto.uc_stack.ss_flags = 0;
    threads[i].contexto.uc_link = &contexto_escalonador;
    makecontext(&threads[i].contexto, executar_thread, 0);

    threads[i].funcao = funcao;
    threads[i].argumento = argumento;
    threads[i].estado = THREAD_PRONTA;
    threads_ativas++;
    return i;
  }
  return -1;
}

void escalonador_executar(void) {
  while (threads_ativas > 0) {
    for (int i = 0; i < ESCALONADOR_MAX_THREADS; i++) {
      if (threads[i].estado != THREAD_PRONTA)
        continue;

      thread_atual = i;
      swapcontext(&contexto_escalonador, &threads[i].contexto);

      if (threads[i].estado == THREAD_FINALIZADA) {
        free(threads[i].pilha);
        threads[i].pilha = NULL;
        threads[i].estado = THREAD_LIVRE;
      }
    }
  }
  thread_atual = -1;
}

void escalonador_ceder(void) {
  if (thread_atual < 0)
    return;
  swapcontext(&threads[thread_atual].contexto, &contexto_escalonador);
}

int escalonador_esperar_semaforo(sem_t *semaforo) {
  while (sem_trywait(semaforo) != 0) {
    if (errno != EAGAIN && errno != EINTR)
      return -1;
    escalonador_ceder();
  }
  return 0;
}

pid_t escalonador_esperar_processo(pid_t pid, int *estado, int opcoes) {
  while (1) {
    pid_t resultado = waitpid(pid, estado, opcoes | WNOHANG);
    if (resultado > 0)
      return resultado;
    if (resultado < 0 && errno != EINTR)
      return -1;
    escalonador_ceder();
  }
}