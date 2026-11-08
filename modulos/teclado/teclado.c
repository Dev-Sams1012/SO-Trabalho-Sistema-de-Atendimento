#include "teclado.h"

#include "atendimento/atendimento.h"
#include "escalonador/escalonador.h"
#include "relogio/relogio.h"

#include <poll.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>

// Intervalo minimo entre duas consultas ao teclado (evita syscall a cada volta).
#define TECLADO_INTERVALO_MS 10

static struct termios terminal_original;
static bool terminal_modificado = false;

void teclado_iniciar(void) {
  if (tcgetattr(STDIN_FILENO, &terminal_original) != 0)
    return;

  struct termios novo = terminal_original;
  novo.c_lflag &= ~(tcflag_t)(ICANON | ECHO); // ISIG continua: Ctrl+C funciona
  novo.c_cc[VMIN] = 0;
  novo.c_cc[VTIME] = 0;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &novo) == 0)
    terminal_modificado = true;
}

void teclado_restaurar(void) {
  if (!terminal_modificado)
    return;
  tcsetattr(STDIN_FILENO, TCSANOW, &terminal_original);
  terminal_modificado = false;
}

// Le o que estiver disponivel. Retorna 1 se encontrou "s", -1 se a entrada
// acabou (EOF) e 0 caso contrario. Nunca bloqueia.
static int procurar_tecla_s(void) {
  struct pollfd entrada = {.fd = STDIN_FILENO, .events = POLLIN, .revents = 0};
  if (poll(&entrada, 1, 0) <= 0 || !(entrada.revents & (POLLIN | POLLHUP)))
    return 0;

  char tecla;
  ssize_t lidos = read(STDIN_FILENO, &tecla, 1);
  if (lidos == 0)
    return -1;
  if (lidos == 1 && (tecla == 's' || tecla == 'S'))
    return 1;
  return 0;
}

void teclado_thread(void *argumento) {
  Atendimento *atendimento = argumento;
  long long ultima_consulta = -TECLADO_INTERVALO_MS;

  while (!atendimento->parar) {
    long long agora = relogio_atual_ms();

    if (agora - ultima_consulta >= TECLADO_INTERVALO_MS) {
      ultima_consulta = agora;

      int resultado = procurar_tecla_s();
      if (resultado == 1) {
        atendimento->parar = 1;
        break;
      }
      if (resultado < 0)
        break; // sem teclado: so SIGINT/SIGTERM encerram
    }

    escalonador_ceder();
  }
}
