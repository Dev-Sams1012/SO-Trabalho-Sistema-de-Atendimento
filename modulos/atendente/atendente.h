#ifndef ATENDENTE_H
#define ATENDENTE_H

// Thread "Atendente": pega o proximo cliente da fila, acorda, espera o
// atendimento terminar, grava o PID no LNG, calcula a satisfacao e acorda o
// analista a cada ANALISTA_MAX_POR_VEZ PIDs gravados.
// "argumento" eh um Atendimento *.
void atendente_thread(void *argumento);

#endif
