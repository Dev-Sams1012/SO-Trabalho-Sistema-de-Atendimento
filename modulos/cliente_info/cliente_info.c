#include "cliente_info.h"

int paciencia_do_cliente(Cliente cliente, int X) {
  switch (cliente.prioridade) {
  case PRIORIDADE_ALTA:
    return X / 2;
  case PRIORIDADE_BAIXA:
    return X;
  }

  // Caso tenha algum problema na inicialização da prioridade do cliente
  return -1;
}
