#include "atendente/atendente.h"
#include "atendimento/atendimento.h"
#include "escalonador/escalonador.h"
#include "recepcao/recepcao.h"
#include "teclado/teclado.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// Converte um texto em inteiro nao negativo. Retorna 0 se for valido.
static int ler_inteiro(const char *texto, int *valor) {
  char *fim;
  errno = 0;
  long convertido = strtol(texto, &fim, 10);

  if (errno != 0 || fim == texto || *fim != '\0' || convertido < 0 ||
      convertido > INT_MAX)
    return -1;

  *valor = (int)convertido;
  return 0;
}

static void mostrar_uso(const char *programa) {
  fprintf(stderr,
          "uso: %s N X\n"
          "  N: numero de clientes (0 = infinitos; tecla 's' encerra)\n"
          "  X: paciencia da prioridade baixa, em ms (> 0)\n",
          programa);
}

int main(int argc, char **argv) {
  int n, x;

  if (argc != 3 || ler_inteiro(argv[1], &n) != 0 ||
      ler_inteiro(argv[2], &x) != 0 || x <= 0) {
    mostrar_uso(argv[0]);
    return 1;
  }

  Atendimento atendimento;
  if (atendimento_iniciar(&atendimento, n, x) != 0)
    return 1;

  if (n == 0) {
    teclado_iniciar();
    atexit(teclado_restaurar);
    printf("Atendimento com clientes infinitos: pressione 's' para encerrar.\n");
    fflush(stdout);
  }

  int criou_threads =
      escalonador_criar_thread(recepcao_thread, &atendimento) >= 0 &&
      escalonador_criar_thread(atendente_thread, &atendimento) >= 0 &&
      (n != 0 ||
       escalonador_criar_thread(teclado_thread, &atendimento) >= 0);

  if (criou_threads) {
    escalonador_executar();
  } else {
    fprintf(stderr, "atendimento: nao foi possivel criar as threads\n");
    atendimento.erro = true;
  }

  teclado_restaurar();
  atendimento_finalizar(&atendimento);
  return atendimento.erro ? 1 : 0;
}
