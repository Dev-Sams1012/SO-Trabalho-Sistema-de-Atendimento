#include "semaforo.h"

#include <fcntl.h>

sem_t *semaforo_criar(const char *nome) {
  sem_unlink(nome); // ignora erro: pode nao existir
  return sem_open(nome, O_CREAT | O_EXCL | O_RDWR, 0644, 1);
}

sem_t *semaforo_abrir(const char *nome) { return sem_open(nome, O_RDWR); }

void semaforo_destruir(const char *nome, sem_t *semaforo) {
  if (semaforo != SEM_FAILED)
    sem_close(semaforo);
  sem_unlink(nome);
}
