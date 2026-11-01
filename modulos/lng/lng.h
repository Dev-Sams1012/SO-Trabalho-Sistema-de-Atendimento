#ifndef LNG_H
#define LNG_H

#include <sys/types.h>

// Arquivos usados pela Lista de Numeros Gerados (ficam no diretorio atual).
#define LNG_ARQUIVO "lng.txt"
#define LNG_ARQUIVO_TEMP "lng.tmp"

// Acrescenta um PID ao FINAL do arquivo LNG (uma linha por PID).
// Retorna 0 em caso de sucesso ou -1 em caso de erro.
int lng_adicionar(pid_t pid);

// Le ate "maximo" PIDs do INICIO do arquivo, guarda-os em "pids" e os apaga
// do arquivo (os que sobraram passam a ser o inicio).
// O vetor "pids" precisa ter espaco para pelo menos "maximo" elementos.
// Retorna quantos PIDs foram lidos (0 se o arquivo nao existe ou esta vazio)
// ou -1 em caso de erro.
//
// IMPORTANTE: este modulo NAO usa semaforo. Quem chama deve estar com o
// semaforo /sem_block fechado.
int lng_ler_remover(int maximo, pid_t *pids);

#endif