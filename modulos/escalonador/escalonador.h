#ifndef ESCALONADOR_H
#define ESCALONADOR_H

#include <semaphore.h>
#include <sys/types.h>

#define ESCALONADOR_MAX_THREADS 8

// Funcao executada por uma thread
typedef void (*FuncaoThread)(void *argumento);

// Prepara o escalonador (deve ser chamado antes de criar threads)
void escalonador_iniciar(void);

// Cria uma thread (contexto proprio com pilha propria).
// Retorna o id da thread ou -1 se nao houver espaco/memoria.
int escalonador_criar_thread(FuncaoThread funcao, void *argumento);

// Executa as threads em round-robin ate que todas terminem.
void escalonador_executar(void);

// Chamada por uma thread para devolver a CPU ao escalonador (troca de contexto)
void escalonador_ceder(void);

// Espera um semaforo abrir SEM bloquear o processo inteiro: enquanto o
// semaforo estiver fechado, cede a CPU para as outras threads.
// Retorna 0 se adquiriu o semaforo ou -1 em caso de erro (errno preenchido).
int escalonador_esperar_semaforo(sem_t *semaforo);

// Espera um processo filho mudar de estado SEM bloquear o processo inteiro
// (usa waitpid com WNOHANG e cede a CPU enquanto nada mudou).
// "opcoes" aceita as mesmas flags do waitpid (ex.: WUNTRACED para esperar o
// filho parar com SIGSTOP). "estado" pode ser NULL.
// Retorna o pid do filho ou -1 em caso de erro (errno preenchido).
pid_t escalonador_esperar_processo(pid_t pid, int *estado, int opcoes);

#endif