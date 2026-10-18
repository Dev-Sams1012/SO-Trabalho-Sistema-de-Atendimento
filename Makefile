# Makefile - Sistema de Atendimento (SO)
#
#   make                  compila atendimento, analista e cliente em build/
#   make test             compila e executa todos os testes de testes/*.c
#   make run N=10 X=500   executa o atendimento com N clientes e paciencia X (ms)
#   make clean            remove build/ (e arquivos temporarios de execucao)

CC      := gcc
CFLAGS  := -std=gnu11 -Wall -Wextra -g -Imodulos -MMD -MP
LDLIBS  := -pthread -lrt

DIR_BUILD := build

# Modulos (tudo em modulos/*/*.c), exceto o analista, que eh um executavel separado.
FONTES_MODULOS := $(filter-out modulos/analista/analista.c,$(wildcard modulos/*/*.c))
OBJ_MODULOS    := $(FONTES_MODULOS:%.c=$(DIR_BUILD)/%.o)

OBJ_MAIN     := $(DIR_BUILD)/src/main.o
OBJ_ANALISTA := $(DIR_BUILD)/modulos/analista/analista.o
OBJ_CLIENTE  := $(DIR_BUILD)/externo/cliente.o

# Testes: cada testes/*.c vira um executavel build/testes/<nome>
FONTES_TESTES := $(wildcard testes/*.c)
BIN_TESTES    := $(FONTES_TESTES:testes/%.c=$(DIR_BUILD)/testes/%)

BIN_ATENDIMENTO := $(DIR_BUILD)/atendimento
BIN_ANALISTA    := $(DIR_BUILD)/analista
BIN_CLIENTE     := $(DIR_BUILD)/cliente

# Parametros padrao para "make run"
N ?= 10
X ?= 1000

.PHONY: all test run clean
.SECONDARY:

all: $(BIN_ATENDIMENTO) $(BIN_ANALISTA) $(BIN_CLIENTE)

$(BIN_ATENDIMENTO): $(OBJ_MAIN) $(OBJ_MODULOS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BIN_ANALISTA): $(OBJ_ANALISTA)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BIN_CLIENTE): $(OBJ_CLIENTE)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

# Regra generica de compilacao: qualquer .c vira build/<mesmo caminho>.o
$(DIR_BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Testes: ligam o teste com todos os modulos (sem main nem analista)
$(DIR_BUILD)/testes/%: $(DIR_BUILD)/testes/%.o $(OBJ_MODULOS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

test: $(BIN_TESTES)
	@for t in $(BIN_TESTES); do \
		echo "== $$t"; \
		./$$t || exit 1; \
	done
	@echo "Todos os testes passaram."

run: all
	./$(BIN_ATENDIMENTO) $(N) $(X)

clean:
	rm -rf $(DIR_BUILD) demanda.txt lng.txt

# Dependencias de cabecalhos geradas automaticamente (-MMD -MP)
-include $(shell find $(DIR_BUILD) -name '*.d' 2>/dev/null)