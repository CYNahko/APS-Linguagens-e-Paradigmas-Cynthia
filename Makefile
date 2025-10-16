# Makefile raiz - Glurr'ik / CocoonVM
CC      = gcc
BISON   = bison -d
FLEX    = flex
CFLAGS  = -g -O0
LDFLAGS = -lfl

SRC_DIR = src
VM_DIR  = vm
BIN_DIR = bin
EX_DIR  = examples

COMPILER = $(BIN_DIR)/glurrikc
VM       = $(BIN_DIR)/cocoonvm

LEXER_SRC  = $(SRC_DIR)/lexer.l
PARSER_SRC = $(SRC_DIR)/parser.y
PARSER_C   = $(SRC_DIR)/parser.tab.c
PARSER_H   = $(SRC_DIR)/parser.tab.h
LEXER_C    = $(SRC_DIR)/lex.yy.c
VM_SRC     = $(VM_DIR)/cocoonvm.c 

.PHONY: all dirs run clean help

all: dirs $(COMPILER) $(VM)

dirs:
	mkdir -p $(BIN_DIR)

# Gera parser.tab.c e parser.tab.h
$(PARSER_C) $(PARSER_H): $(PARSER_SRC)
	$(BISON) -o $(PARSER_C) $(PARSER_SRC)

# Gera lex.yy.c (depende do header do bison)
$(LEXER_C): $(LEXER_SRC) $(PARSER_H)
	$(FLEX) -o $(LEXER_C) $(LEXER_SRC)

# Compila o compilador
$(COMPILER): $(PARSER_C) $(LEXER_C)
	$(CC) $(CFLAGS) -o $(COMPILER) $(PARSER_C) $(LEXER_C) $(LDFLAGS)

# Compila a VM
$(VM): $(VM_SRC)
	$(CC) $(CFLAGS) -o $(VM) $(VM_SRC)

run: all
	@echo "Compilando exemplo..."
	$(COMPILER) $(EX_DIR)/metamorfose.glk > $(EX_DIR)/metamorfose.casm
	@echo "Executando na VM..."
	$(VM) $(EX_DIR)/metamorfose.casm

clean:
	rm -f $(SRC_DIR)/parser.tab.* $(SRC_DIR)/lex.yy.c
	rm -rf $(BIN_DIR)

help:
	@echo "Comandos disponíveis:"
	@echo "  make            - Compila compilador e VM"
	@echo "  make run        - Compila e executa o exemplo padrão"
	@echo "  make clean      - Remove arquivos gerados"
