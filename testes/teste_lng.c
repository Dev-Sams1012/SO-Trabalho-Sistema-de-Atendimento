#include "lng/lng.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define VERIFICAR(condicao)                                                    \
  do {                                                                         \
    if (!(condicao)) {                                                         \
      fprintf(stderr, "FALHOU %s:%d: %s\n", __FILE__, __LINE__, #condicao);    \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

static void apagar_arquivos(void) {
  unlink(LNG_ARQUIVO);
  unlink(LNG_ARQUIVO_TEMP);
}

int main(void) {
  pid_t pids[10];

  printf("teste_lng\n");
  apagar_arquivos();

  // 1. Arquivo que ainda nao existe: nao eh erro, apenas nao ha nada.
  VERIFICAR(lng_ler_remover(10, pids) == 0);
  printf("  ok: arquivo inexistente devolve 0\n");

  // 2. Grava 25 PIDs (1000 a 1024).
  for (int i = 0; i < 25; i++)
    VERIFICAR(lng_adicionar(1000 + i) == 0);

  // 3. Primeira leitura: os 10 primeiros, na ordem em que foram gravados.
  VERIFICAR(lng_ler_remover(10, pids) == 10);
  for (int i = 0; i < 10; i++)
    VERIFICAR(pids[i] == 1000 + i);

  // 4. Segunda leitura: os 10 seguintes (os primeiros foram apagados).
  VERIFICAR(lng_ler_remover(10, pids) == 10);
  for (int i = 0; i < 10; i++)
    VERIFICAR(pids[i] == 1010 + i);

  // 5. Terceira leitura: so restam 5.
  VERIFICAR(lng_ler_remover(10, pids) == 5);
  for (int i = 0; i < 5; i++)
    VERIFICAR(pids[i] == 1020 + i);

  // 6. Agora esta vazio.
  VERIFICAR(lng_ler_remover(10, pids) == 0);
  printf("  ok: 25 PIDs lidos de 10 em 10 (10 + 10 + 5), em ordem, sem perda\n");

    // 7. Le apenas 2 e depois acrescenta mais um: a ordem deve ser preservada.
  VERIFICAR(lng_adicionar(1) == 0);
  VERIFICAR(lng_adicionar(2) == 0);
  VERIFICAR(lng_adicionar(3) == 0);
  VERIFICAR(lng_ler_remover(2, pids) == 2);
  VERIFICAR(pids[0] == 1 && pids[1] == 2);
  VERIFICAR(lng_adicionar(4) == 0);
  VERIFICAR(lng_ler_remover(10, pids) == 2);
  VERIFICAR(pids[0] == 3 && pids[1] == 4);
  printf("  ok: leitura parcial e nova escrita mantem a ordem\n");

  apagar_arquivos();
  printf("teste_lng: tudo certo\n");
  return 0;
}