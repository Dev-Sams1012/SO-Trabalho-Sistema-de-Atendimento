#ifndef CLIENTE_INFO_H
#define CLIENTE_INFO_H

#include <sys/types.h>

typedef enum { PRIORIDADE_BAIXA, PRIORIDADE_ALTA } PRIORIDADE;

typedef struct {
  pid_t pid;
  long long hora_chegada; // ms desde o inicio do programa
  PRIORIDADE prioridade;  // PRIORIDADE_ALTA ou PRIORIDADE_BAIXA
  int tempo_atendimento;  // lido do arquivo demanda
} Cliente;

int paciencia_do_cliente(Cliente cliente, int X);

#endif
