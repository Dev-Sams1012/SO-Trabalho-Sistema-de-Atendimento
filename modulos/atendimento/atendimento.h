#ifndef ATENDIMENTO_H
#define ATENDIMENTO_H

#include "fila/fila.h"

#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>

// Arquivo compartilhado entre o cliente (escreve) e a recepcao (le).
#define ATENDIMENTO_ARQUIVO_DEMANDA "demanda.txt"

// Estado compartilhado do processo Atendimento. Todas as threads recebem um
// ponteiro para esta estrutura. Como as threads sao cooperativas (so trocam de
// contexto em escalonador_ceder), nao ha corrida entre elas.
typedef struct {
  int n; // numero de clientes (0 = infinitos)
  int x; // paciencia da prioridade baixa, em ms

  Fila fila;
  sem_t *sem_atend;
  sem_t *sem_block;
  pid_t pid_analista;

  volatile sig_atomic_t parar; // pedido de parada (tecla "s", SIGINT, erro)
  bool recepcao_terminou;      // a recepcao nao vai mais inserir clientes
  bool erro;                   // houve falha em alguma thread

  int criados;       // clientes criados e inseridos na fila
  int atendidos;     // clientes cujo atendimento foi concluido
  int satisfeitos;   // atendidos dentro da paciencia
  int pendentes_lng; // PIDs gravados no LNG que o analista ainda nao imprimiu
} Atendimento;

// Prepara tudo antes das threads: relogio, escalonador, fila, semaforos,
// analista (criado dormindo) e tratamento de SIGINT/SIGTERM.
// Retorna 0 em caso de sucesso ou -1 em caso de erro (ja limpou o que criou).
int atendimento_iniciar(Atendimento *atendimento, int n, int x);

// Acorda o analista e espera ele voltar a dormir (sem travar as outras
// threads). Atualiza pendentes_lng.
void atendimento_acordar_analista(Atendimento *atendimento);

// Esvazia o LNG (acordando o analista quantas vezes forem necessarias),
// imprime a taxa de satisfacao e o tempo total, e libera tudo (clientes que
// sobraram na fila, analista, semaforos e arquivos temporarios).
void atendimento_finalizar(Atendimento *atendimento);

#endif
