#include "relogio.h"

static struct timespec tempo_inicio;

void relogio_iniciar(void) { clock_gettime(CLOCK_MONOTONIC, &tempo_inicio); }

long long relogio_atual_ms(void) {
  struct timespec tempo_atual;
  clock_gettime(CLOCK_MONOTONIC, &tempo_atual);

  long long delta_ns =
      (long long)(tempo_atual.tv_sec - tempo_inicio.tv_sec) * 1000000000LL +
      (tempo_atual.tv_nsec - tempo_inicio.tv_nsec);

  return delta_ns / 1000000;
}
