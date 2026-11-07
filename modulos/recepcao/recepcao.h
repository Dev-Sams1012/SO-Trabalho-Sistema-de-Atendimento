#ifndef RECEPCAO_H
#define RECEPCAO_H

// Executavel do cliente (gerado pelo Makefile).
#define CLIENTE_CAMINHO "./build/cliente"

// Thread "Recepcao": cria os processos cliente (N ou infinitos), espera cada
// um parar (SIGSTOP) depois de gravar a demanda, sorteia a prioridade (50% /
// 50%) e insere na fila. Pausa a criacao enquanto a fila estiver cheia.
// "argumento" eh um Atendimento *.
void recepcao_thread(void *argumento);

#endif
