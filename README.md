# SO-Trabalho-Sistema-de-Atendimento

Este repositório contém o trabalho da parte prática da disciplina Sistemas Operacionais, ministrada pelo professor Yuri Lenon no semestre 2026.2.

## Sobre o trabalho

Sistema de atendimento em C para Linux. Uma thread **Recepção** cria processos "cliente" e os coloca numa fila; uma thread **Atendente** os atende (prioridade alta primeiro); e um processo **Analista** imprime os PIDs dos clientes já atendidos. Ao final, o programa mostra a taxa de satisfação e o tempo total de execução.

As threads são feitas com troca de contexto, usando um escalonador próprio, sem bibliotecas de paralelismo.

## Módulos

Ficam na pasta `modulos/`. O ponto de entrada é o `main.c`, e o `externo/cliente.c` é o processo cliente fornecido pelo professor Yuri.

| Módulo | Para que serve |
|---|---|
| `atendimento` | estado compartilhado, preparação inicial (semáforos, analista) e encerramento com o resultado final |
| `recepcao` | thread que cria os clientes e os coloca na fila |
| `atendente` | thread que atende os clientes e registra o PID de cada um |
| `teclado` | thread que espera a tecla `s` para encerrar quando N = 0 |
| `analista` | processo que dorme e, quando acordado, imprime até 10 PIDs |
| `escalonador` | threads por troca de contexto (round-robin) |
| `fila` | fila de clientes, uma por prioridade |
| `lng` | arquivo com a Lista de Números Gerados (PIDs atendidos) |
| `semaforo` | criação e remoção dos semáforos `/sem_atend` e `/sem_block` |
| `relogio` | tempo em milissegundos desde o início do programa |
| `cliente_info` | dados do cliente e cálculo da sua paciência |

## Como usar

Rode sempre da raiz do projeto (precisa de Linux, `gcc` e `make`).

```bash
make                    # compila tudo em build/
make run N=50 X=500     # atende 50 clientes; X é a paciência (ms) da prioridade baixa
make run N=0 X=500      # clientes infinitos; aperte 's' para encerrar
make test               # roda todos os testes
make clean              # remove build/ e arquivos temporários
```

## Equipe

- Jonathan Duarte Uchôa - 565636
- Samuel Augusto de Abreu - 565164
