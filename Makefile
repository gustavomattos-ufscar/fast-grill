# =====================================================================
#  Makefile do Fast Grill
#
#  make          -> compila o jogo com janela (precisa da raylib)
#  make texto    -> compila o jogo em modo texto (sem raylib)
#  make test     -> compila e roda todos os testes (sem janela)
#  make capturas -> gera imagens das telas em docs/ (evidencias)
#  make clean    -> apaga os executaveis
#
#  No Windows, use o "make" do w64devkit (instalado pelo
#  instalar_ferramentas.ps1). Se a raylib estiver em outra pasta:
#     make RAYLIB=C:/caminho/da/raylib
# =====================================================================

CC     = gcc
# -std=c99: padrao da linguagem usado na disciplina
# -Wall -Wextra: liga os avisos (queremos compilar SEM nenhum aviso)
# -g: inclui informacoes para depuracao (gdb)
CFLAGS = -std=c99 -Wall -Wextra -g

# Modulos de regra e estruturas (nao dependem da raylib)
NUCLEO = src/pedido.c src/fila.c src/pilha.c src/jogo.c
HEADERS = src/pedido.h src/fila.h src/pilha.h src/jogo.h

# Diferencas entre Windows e Linux
ifeq ($(OS),Windows_NT)
    EXE = .exe
    RAYLIB ?= C:/dev/raylib-5.5_win64_mingw-w64
    RAYLIB_FLAGS = -I$(RAYLIB)/include -L$(RAYLIB)/lib -lraylib -lopengl32 -lgdi32 -lwinmm
else
    EXE =
    RAYLIB_FLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

JOGO  = fast-grill$(EXE)
TEXTO = fast-grill-texto$(EXE)
TESTES = tests/teste_fila$(EXE) tests/teste_pilha$(EXE) tests/teste_jogo$(EXE)

.PHONY: all texto test capturas clean

# --- jogo com janela (raylib) ---------------------------------------
all: $(JOGO)

# [v2] som.c (efeitos sonoros) entrou no jogo com janela
$(JOGO): $(NUCLEO) $(HEADERS) src/ui.c src/ui.h src/som.c src/som.h src/main.c
	$(CC) $(CFLAGS) $(NUCLEO) src/ui.c src/som.c src/main.c -o $@ $(RAYLIB_FLAGS)

# --- jogo em modo texto (terminal) ----------------------------------
texto: $(TEXTO)

$(TEXTO): $(NUCLEO) $(HEADERS) src/main_texto.c
	$(CC) $(CFLAGS) $(NUCLEO) src/main_texto.c -o $@

# --- testes -----------------------------------------------------------
tests/teste_fila$(EXE): tests/teste_fila.c tests/teste.c tests/teste.h src/fila.c src/pedido.c $(HEADERS)
	$(CC) $(CFLAGS) tests/teste_fila.c tests/teste.c src/fila.c src/pedido.c -o $@

tests/teste_pilha$(EXE): tests/teste_pilha.c tests/teste.c tests/teste.h src/pilha.c $(HEADERS)
	$(CC) $(CFLAGS) tests/teste_pilha.c tests/teste.c src/pilha.c -o $@

tests/teste_jogo$(EXE): tests/teste_jogo.c tests/teste.c tests/teste.h $(NUCLEO) $(HEADERS)
	$(CC) $(CFLAGS) tests/teste_jogo.c tests/teste.c $(NUCLEO) -o $@

# Roda os tres programas de teste; se algum falhar, o make para com erro.
test: $(TESTES)
	./tests/teste_fila$(EXE)
	./tests/teste_pilha$(EXE)
	./tests/teste_jogo$(EXE)

# Abre o jogo no modo --captura (joga sozinho e salva as telas).
capturas: $(JOGO)
	./$(JOGO) --captura
	mkdir -p docs
	mv captura_*.png docs/

clean:
	rm -f $(JOGO) $(TEXTO) $(TESTES)
