# Fast Grill

Jogo da disciplina **Estruturas de Dados (1001098) — UFSCar Sorocaba, 2026/2**.

Você é o chapeiro de uma lanchonete. Os clientes formam uma **fila** e só o da frente pode ser atendido. O lanche é montado em uma **pilha**, da base para o topo, na ordem exata da receita. Cliente que espera demais, ou que encontra a fila lotada, vai embora e você perde uma vida.

Antes de jogar, escolha a dificuldade (**Fácil**, **Médio** ou **Difícil**). O jogo tem efeitos sonoros.

> Para entender o código por dentro, leia o **[entenda.md](entenda.md)**. Para ver o que mudou em cada versão, leia o **[MUDANCAS.md](MUDANCAS.md)**.

## Equipe

| RA | Nome |
|---|---|
| 858177 | Gustavo Rodrigues Cesar de Mattos |
| 856527 | Lucas Soares Nery Nobre |
| 859432 | Mateus da Silva Baú |

## Requisitos

- **gcc** e **make**
- **raylib 5.5**, só para a versão com janela. O modo texto e os testes não precisam dela.

No Windows, dá para instalar tudo de uma vez, sem administrador, com o script da pasta do projeto:

```
powershell -ExecutionPolicy Bypass -File instalar_ferramentas.ps1
```

Ele instala o w64devkit (gcc + make) e a raylib 5.5 em `C:\dev`. Depois, feche e abra o terminal.

No Linux, instale a raylib pelo gerenciador de pacotes ou compile-a pelo site oficial.

## Como compilar e executar

Dentro da pasta `fast-grill`:

| Comando | O que faz |
|---|---|
| `make` | compila o jogo com janela, gerando `fast-grill.exe` |
| `make texto` | compila o jogo no terminal, gerando `fast-grill-texto.exe` |
| `make test` | compila e roda **todos os testes** (187: fila, pilha e regras) |
| `make capturas` | gera imagens das telas em `docs/` |
| `make clean` | apaga os executáveis |

Se a raylib estiver em outra pasta: `make RAYLIB=C:/caminho/da/raylib`.

Para rodar:

```
./fast-grill.exe                 # jogo com janela
./fast-grill-texto.exe           # jogo no terminal (semente aleatória)
./fast-grill-texto.exe 42        # semente fixa: sempre a mesma partida (Fácil)
./fast-grill-texto.exe 42 dificil   # semente fixa no Difícil (facil | medio | dificil)
```

## Dificuldades

| | Fácil | Médio | Difícil |
|---|---|---|---|
| Ingredientes | 5 | 7 (+Bacon, Ovo) | 8 (+Cebola) |
| Paciência (receita de 4 itens) | 20 s | 14 s | 13 s |
| Novo cliente a cada | 10 s (até 4 s) | 8 s (até 3 s) | 6 s (até 3 s) |
| Fila lota com | 5 | 5 | 4 |
| Servir errado | sem castigo | −2 de paciência | −3 de paciência |
| Pontos por ingrediente | 10 | 15 | 20 |

## Controles

| Janela | Terminal | Ação |
|---|---|---|
| `1` `2` `3`, setas ou clique (menu) | 2º argumento | escolhe a dificuldade |
| `ENTER` (menu) | — | começa a partida |
| `1` a `8` ou clique | `1` a `8` | empilha um ingrediente (5, 7 ou 8, conforme a dificuldade) |
| `Z` ou `Backspace` | `d` | desfaz (tira o ingrediente do topo) |
| `Espaço` | `s` | serve o cliente da frente |
| (tempo real, 1 tick/s) | `t` ou `t 5` | passa 1 (ou 5) ticks |
| `ENTER` (fim) | — | joga de novo, na mesma dificuldade |
| `Backspace` (fim) | — | volta ao menu |
| `M` | — | liga/desliga o som |
| `ESC` | `q` | sai |

## Organização

```
fast-grill/
├── src/
│   ├── pedido.h/.c     tipos Ingrediente, Receita, Pedido + cardápio + sorteio
│   ├── fila.h/.c       TAD Fila de Pedidos   (vetor circular)       <- feito pelo grupo
│   ├── pilha.h/.c      TAD Pilha de Prato    (vetor)                <- feito pelo grupo
│   ├── jogo.h/.c       regras da partida (tick, servir, vidas, dificuldades...)
│   ├── ui.h/.c         desenho das telas (usa raylib)
│   ├── som.h/.c        efeitos sonoros gerados por código (usa raylib)
│   ├── main.c          janela + laço principal (usa raylib)
│   └── main_texto.c    versão de terminal (sem raylib)
├── assets/             fonte Fredoka (.ttf) + licença OFL; fica ao lado do .exe
├── tests/              testes sem janela (make test)
├── docs/               capturas de tela e saídas usadas no relatório
└── Makefile
```

**Código externo:** só a biblioteca [raylib 5.5](https://www.raylib.com/), usada **apenas** em `ui.c`, `som.c` e `main.c`. Todo o resto, incluindo os TADs Fila e Pilha, foi escrito para este projeto. Os sons não vêm de arquivos de terceiros: são gerados pelo `som.c`. A fonte dos textos é a [Fredoka](https://fonts.google.com/specimen/Fredoka) (licença livre SIL OFL 1.1, arquivo em `assets/`); se a pasta `assets/` faltar, o jogo usa a fonte padrão da raylib.
