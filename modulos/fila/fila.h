#ifndef FILA_H
#define FILA_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define FILA_CAPACIDADE 100

#define PRIORIDADE_BAIXA 0
#define PRIORIDADE_ALTA 1

typedef struct {
  pid_t pid;
  long long hora_chegada; // ms desde o inicio do programa
  int prioridade;         // PRIORIDADE_ALTA ou PRIORIDADE_BAIXA
  int tempo_atendimento;  // lido do arquivo demanda
} Cliente;

// Fila circular simples (vetor + indice de inicio + quantidade)
typedef struct {
  Cliente clientes[FILA_CAPACIDADE];
  int inicio;
  int tamanho;
} FilaCircular;

// Duas filas: uma para cada prioridade. O atendimento sempre olha primeiro a
// fila de prioridade alta.
typedef struct {
  FilaCircular alta;
  FilaCircular baixa;
} Fila;

void fila_iniciar(Fila *fila);

// Insere no fim da fila correspondente a prioridade do cliente.
// Retorna 0 em caso de sucesso, -1 se a fila estiver cheia.
int fila_inserir(Fila *fila, Cliente cliente);

// Retorna o proximo cliente a ser atendido SEM remove-lo (ou NULL se vazia).
// Alta prioridade primeiro; dentro de cada prioridade, ordem de chegada.
Cliente *fila_espiar(Fila *fila);

// Remove o cliente que fila_espiar retornaria. Retorna 0 se removeu
// corretamente ou -1 se vazia.
int fila_remover(Fila *fila);

// Quantidade total de clientes esperando (alta + baixa)
int fila_tamanho(const Fila *fila);

bool fila_vazia(const Fila *fila);
bool fila_cheia(const Fila *fila);

#endif