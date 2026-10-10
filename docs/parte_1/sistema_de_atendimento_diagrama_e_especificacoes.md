# Sistema de Atendimento - Especificações e Arquitetura

Este documento consolida todas as informações, estruturas de dados, processos e notas descritas no diagrama do sistema de atendimento.

---

## 1. Estrutura de Dados do Cliente na Fila
- **PID**
- **Hora de chegada** (Tempo em ms passados entre o início do programa e a criação do cliente)
- **Prioridade**
- **Tempo para atendimento**

---

## 2. Fila de Clientes
- **Fila de clientes** (A estrutura de dados da fila é uma escolha de projeto, podendo ser mais de uma)

---

## 3. Processos e Threads

### Processo 1 - Atendimento
*O processo Atendimento recebe de entrada o valor $N$ de número de clientes a serem criados e o valor $X$ de paciência de prioridade baixa. Ele dá de saída (imprime na tela) a taxa de satisfação e o tempo total de execução. Termina quando todos os $N$ clientes forem atendidos.*

#### Thread 1 - Atendente
- Lê na fila o próximo cliente
- Acorda o cliente
- Espera semáforo `\sem_atend` abrir
- Espera semáforo `\sem_block` abrir
- Fecha semáforo `\sem_block`
- Escreve o número do cliente (PID) no arquivo Lista de Números Gerados (LNG)
- Abre semáforo `\sem_block`
- Calcula a satisfação do cliente (Satisfeito se $\text{tempo atual} - \text{hora de chegada} \le \text{paciência}$; caso contrário, Insatisfeito)
- Tira cliente da fila
- Acorda o Analista

#### Thread 2 - Recepção
- Cria semáforos `\sem_atend` e `\sem_block`
- Cria $N$ ou infinitos processos clientes (se $N = 0$, a Recepção fica em loop infinito criando clientes e pausa a criação se a fila atingir 100 clientes em espera, retomando quando houver espaço)
- Cada processo é criado com uma prioridade entre duas possíveis (prioridade aleatória: 50% para cada)

---

### Processo 2 - Analista
- Gera arquivo com seu PID e dorme
- Quando acordado:
  - Bloqueia LNG
  - Lê LNG e imprime seus 10 primeiros valores
  - Apaga os 10 valores que imprimiu
  - Desbloqueia LNG
- Dorme novamente

---

### Processo 3 - Cliente
- Gera e escreve seu tempo necessário para atendimento no arquivo **Demanda**
- Dorme
- Quando acordado: 
  - Fecha o semáforo de atendimento `\sem_atend`
  - Pausa pela quantidade de tempo de atendimento calculado
  - Abre o semáforo `\sem_atend` de atendimento
- Finaliza

---

## 4. Arquivos do Sistema

### Arquivo LNG
- Bloqueável pelo semáforo `\sem_block`

### Arquivo Demanda
- Escrito pelo cliente na geração
- Lido pelo Atendimento para colocar na fila
- Após lido, pode ser apagado
- *Nota:* O arquivo Demanda pode ser único e serve para comunicação entre os processos "Cliente" e "Atendimento" apenas no momento da inserção na fila. Como os clientes são colocados na fila um por vez, esse arquivo é compartilhado entre todos os clientes.

---

## 5. Notas e Regras de Projeto

1. **Estratégia do Analista:** A estratégia para acordar o analista (a cada atendimento, periódico por tempo, por quantidade de atendimentos ou qualquer outra estratégia) é de escolha livre.
2. **Fila e Inserção:** A estrutura de dados da "fila" é uma escolha de projeto. Não precisa ser literalmente uma fila e pode ser mais de uma. A inserção de clientes na fila pode ser feita pela thread 2 ou por uma nova thread criada especificamente para o tratamento da fila.
3. **Prioridade e Paciência:** 
   - **Prioridade baixa:** Cliente tem $X$ ms de "paciência".
   - **Prioridade alta:** Cliente tem $X/2$ ms de "paciência".
   - O valor de $X$ é dado de entrada para o Processo "Atendimento".
4. **Hora de Chegada:** Tempo em ms passados entre o início do programa e a criação do cliente.
5. **Processo Cliente:** Fornecido previamente. Todas as equipes utilizarão o mesmo processo cliente.
6. **Satisfação do Cliente:** Satisfeito se $(\text{tempo atual} - \text{hora de chegada}) \le \text{paciência}$. Caso contrário, é "Insatisfeito".
7. **Arquivo Demanda:** Pode ser único e serve para comunicação entre os processos "Cliente" e "Atendimento" apenas no momento da inserção na fila. Como os clientes são colocados na fila um por vez, esse arquivo é compartilhado entre todos os clientes.
8. **Parâmetros do Atendimento:** O processo Atendimento recebe de entrada o valor $N$ (número de clientes) e o valor $X$ (paciência de prioridade baixa). Dá de saída (imprime na tela) a taxa de satisfação e o tempo total de execução, terminando quando todos os $N$ clientes forem atendidos.
9. **Comportamento da Recepção com $N = 0$:** Quando o $N$ passado for 0, a Recepção deve ficar em loop infinito criando clientes. Deve pausar a criação se a fila ficar com 100 clientes em espera, voltando a criar mais toda vez que um espaço for liberado na fila.