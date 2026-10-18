#ifndef RELOGIO_H
#define RELOGIO_H

#include <time.h>

// Inicializa o tempo de referência (início do programa)
void relogio_iniciar(void);

// Retorna o tempo decorrido em milissegundos desde a inicialização
long long relogio_atual_ms(void);

#endif
