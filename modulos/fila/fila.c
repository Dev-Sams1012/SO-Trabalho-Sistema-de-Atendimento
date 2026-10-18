#include "fila.h"

static void circular_iniciar(FilaCircular *fila) {
  fila->inicio = 0;
  fila->tamanho = 0;
}

static void circular_inserir(FilaCircular *fila, Cliente cliente) {
  int posicao = (fila->inicio + fila->tamanho) % FILA_CAPACIDADE;
  fila->clientes[posicao] = cliente;
  fila->tamanho++;
}

static Cliente *circular_frente(FilaCircular *fila) {
  if (fila->tamanho == 0)
    return NULL;
  return &fila->clientes[fila->inicio];
}

static void circular_remover(FilaCircular *fila) {
  fila->inicio = (fila->inicio + 1) % FILA_CAPACIDADE;
  fila->tamanho--;
}

void fila_iniciar(Fila *fila) {
  circular_iniciar(&fila->alta);
  circular_iniciar(&fila->baixa);
}

int fila_inserir(Fila *fila, Cliente cliente) {
  if (fila_cheia(fila))
    return -1;

  if (cliente.prioridade == PRIORIDADE_ALTA)
    circular_inserir(&fila->alta, cliente);
  else
    circular_inserir(&fila->baixa, cliente);

  return 0;
}

Cliente *fila_espiar(Fila *fila) {
  Cliente *cliente = circular_frente(&fila->alta);
  if (cliente != NULL)
    return cliente;
  return circular_frente(&fila->baixa);
}

int fila_remover(Fila *fila) {
  if (fila->alta.tamanho > 0) {
    circular_remover(&fila->alta);
    return 0;
  }
  if (fila->baixa.tamanho > 0) {
    circular_remover(&fila->baixa);
    return 0;
  }
  return -1;
}

int fila_tamanho(const Fila *fila) {
  return fila->alta.tamanho + fila->baixa.tamanho;
}

bool fila_vazia(const Fila *fila) { return fila_tamanho(fila) == 0; }

bool fila_cheia(const Fila *fila) {
  return fila_tamanho(fila) >= FILA_CAPACIDADE;
}