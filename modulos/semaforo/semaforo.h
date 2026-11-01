#ifndef SEMAFORO_H
#define SEMAFORO_H

#include <semaphore.h>

#define SEMAFORO_NOME_ATEND "/sem_atend"
#define SEMAFORO_NOME_BLOCK "/sem_block"

// Remove um semaforo antigo (se sobrou de execucao anterior) e cria um novo
// com valor inicial 1. Retorna SEM_FAILED em caso de erro.
sem_t *semaforo_criar(const char *nome);

// Abre um semaforo que ja existe (usado por quem nao eh a recepcao,
// ex.: o analista). Retorna SEM_FAILED em caso de erro.
sem_t *semaforo_abrir(const char *nome);

// Fecha e remove o semaforo do sistema.
void semaforo_destruir(const char *nome, sem_t *semaforo);

#endif
