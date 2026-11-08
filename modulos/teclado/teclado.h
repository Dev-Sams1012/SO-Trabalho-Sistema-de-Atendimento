#ifndef TECLADO_H
#define TECLADO_H

// Deixa o terminal sem eco e sem esperar ENTER, para a tecla "s" ser lida na
// hora. Se a entrada padrao nao for um terminal, nao faz nada.
void teclado_iniciar(void);

// Devolve o terminal ao modo original (seguro chamar varias vezes).
void teclado_restaurar(void);

// Thread "Teclado" (so usada com N = 0): sem bloquear, procura a tecla "s" e,
// ao encontrar, pede a parada do Atendimento.
// "argumento" eh um Atendimento *.
void teclado_thread(void *argumento);

#endif
