#include "lng.h"

#include <errno.h>
#include <stdio.h>

int lng_adicionar(pid_t pid) {
  FILE *arquivo = fopen(LNG_ARQUIVO, "a");
  if (arquivo == NULL)
    return -1;

  int escrita = fprintf(arquivo, "%d\n", (int)pid);
  int fechamento = fclose(arquivo);

  if (escrita < 0 || fechamento != 0)
    return -1;
  return 0;
}

int lng_ler_remover(int maximo, pid_t *pids) {
  FILE *origem = fopen(LNG_ARQUIVO, "r");
  if (origem == NULL)
    return (errno == ENOENT) ? 0 : -1;

  int lidos = 0;
  int valor;
  while (lidos < maximo && fscanf(origem, "%d", &valor) == 1)
    pids[lidos++] = (pid_t)valor;
    
  FILE *destino = fopen(LNG_ARQUIVO_TEMP, "w");
  if (destino == NULL) {
    fclose(origem);
    return -1;
  }

  while (fscanf(origem, "%d", &valor) == 1)
    fprintf(destino, "%d\n", valor);

  fclose(origem);
  if (fclose(destino) != 0 || rename(LNG_ARQUIVO_TEMP, LNG_ARQUIVO) != 0)
    return -1;

  return lidos;
}