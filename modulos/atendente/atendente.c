#include "atendente.h"

#include "analista/analista.h"
#include "atendimento/atendimento.h"
#include "escalonador/escalonador.h"
#include "lng/lng.h"
#include "relogio/relogio.h"

#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

// Grava o PID no LNG com /sem_block fechado, sem travar as outras threads.
static void registrar_no_lng(Atendimento *atendimento, pid_t pid) {
  if (escalonador_esperar_semaforo(atendimento->sem_block) != 0) {
    perror("atendente: esperar /sem_block");
    atendimento->erro = true;
    return;
  }

  int resultado = lng_adicionar(pid);
  sem_post(atendimento->sem_block);

  if (resultado == 0)
    atendimento->pendentes_lng++;
  else
    atendimento->erro = true;
}

// Atende um cliente que ja foi tirado da fila.
static void atender(Atendimento *atendimento, Cliente cliente) {
  if (kill(cliente.pid, SIGCONT) != 0) {
    perror("atendente: acordar cliente");
    return;
  }

  // Esperar so o /sem_atend nao basta: logo apos o SIGCONT ele ainda esta
  // aberto (o cliente nao chegou no sem_wait). O fim do processo garante que
  // o atendimento acabou e ainda recolhe o zumbi.
  if (escalonador_esperar_processo(cliente.pid, NULL, 0) != cliente.pid) {
    perror("atendente: esperar cliente");
    return;
  }
  long long fim = relogio_atual_ms();

  registrar_no_lng(atendimento, cliente.pid);

  atendimento->atendidos++;
  if (fim - cliente.hora_chegada <=
      paciencia_do_cliente(cliente, atendimento->x))
    atendimento->satisfeitos++;

  if (atendimento->pendentes_lng >= ANALISTA_MAX_POR_VEZ)
    atendimento_acordar_analista(atendimento);
}

void atendente_thread(void *argumento) {
  Atendimento *atendimento = argumento;

  while (!atendimento->parar) {
    Cliente *proximo = fila_espiar(&atendimento->fila);

    if (proximo == NULL) {
      if (atendimento->recepcao_terminou)
        break;
      escalonador_ceder();
      continue;
    }

    // Tira da fila ANTES de atender: durante o atendimento a recepcao pode
    // inserir um cliente de prioridade alta, e a remocao tiraria o errado.
    Cliente cliente = *proximo;
    fila_remover(&atendimento->fila);

    atender(atendimento, cliente);
  }
}
