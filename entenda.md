# Entenda o Fast Grill

Guia para o grupo entender **todo** o código: o que cada arquivo faz, como as peças se conectam e por que fizemos cada escolha. Leiam na ordem; cada parte usa a anterior.

> **Dica:** abram o arquivo `.c` correspondente ao lado enquanto leem cada seção. Todo o código está comentado linha a linha.
>
> **[v2] Já leram a versão 1?** Leiam o [MUDANCAS.md](MUDANCAS.md) e, aqui, as partes marcadas com **[v2]**: seções 4, 7, 8, as novas **9 (Dificuldade)** e **10 (Sons)**, e a 12. No código, toda mudança tem um comentário `[v2]`.

---

## Sumário

1. [O jogo em 1 minuto](#1-o-jogo-em-1-minuto)
2. [Como rodar](#2-como-rodar)
3. [O mapa do projeto: 3 camadas](#3-o-mapa-do-projeto-3-camadas)
4. [Os tipos básicos — `pedido.h/.c`](#4-os-tipos-básicos--pedidohc)
5. [TAD Fila de Pedidos — `fila.h/.c`](#5-tad-fila-de-pedidos--filahc)
6. [TAD Pilha de Prato — `pilha.h/.c`](#6-tad-pilha-de-prato--pilhahc)
7. [O núcleo do jogo — `jogo.h/.c`](#7-o-núcleo-do-jogo--jogohc)
8. [A interface — `ui.c` e `main.c`](#8-a-interface--uic-e-mainc)
9. [[v2] As dificuldades](#9-v2-as-dificuldades)
10. [[v2] Os efeitos sonoros — `som.h/.c`](#10-v2-os-efeitos-sonoros--somhc)
11. [O modo texto — `main_texto.c`](#11-o-modo-texto--main_textoc)
12. [Os testes — `tests/`](#12-os-testes--tests)
13. [Uma tecla, do começo ao fim](#13-uma-tecla-do-começo-ao-fim)
14. [Perguntas que o professor pode fazer](#14-perguntas-que-o-professor-pode-fazer)
15. [Como mexer no código](#15-como-mexer-no-código)
16. [O que fica para a Fase 3](#16-o-que-fica-para-a-fase-3)
17. [Glossário](#17-glossário)

---

## 1. O jogo em 1 minuto

O jogador é o chapeiro de uma lanchonete.

- Os **clientes chegam e formam uma fila**. Só o cliente da **frente** pode ser atendido. → **FILA (FIFO)**
- Cada cliente pede um lanche com uma **receita em ordem fixa** (ex.: Pão, Carne, Queijo, Pão).
- O jogador monta o lanche **empilhando** ingredientes no prato, da base para o topo. Errou? **Desfazer** tira só o **último** ingrediente colocado. → **PILHA (LIFO)**
- **Servir** só funciona se o prato for **idêntico** à receita, na mesma ordem. Cada ingrediente vale pontos.
- O tempo anda em **ticks** (1 por segundo). Só o cliente da frente perde paciência.
- **Perde 1 vida** quando o cliente da frente desiste ou quando chega um cliente com a fila **lotada**.
- Com **0 vidas**, a partida acaba.
- A cada alguns pedidos servidos, sobe de **nível**: clientes chegam mais rápido e aparecem receitas maiores.
- **[v2]** Antes de jogar, escolhe-se a **dificuldade** (Fácil, Médio ou Difícil), e o jogo tem **efeitos sonoros**.

**As duas estruturas de dados não são enfeite: elas SÃO as regras do jogo.** "Só atende a frente" é a fila. "Desfazer tira o último" é a pilha.

---

## 2. Como rodar

(Detalhes no [README.md](README.md).)

```
make test                       # roda os 187 testes: tem que aparecer "TUDO CERTO" 3 vezes
make                            # compila o jogo com janela
./fast-grill.exe                # joga!
make texto                      # versão de terminal
./fast-grill-texto.exe 42       # partida fixa (semente 42), Fácil
./fast-grill-texto.exe 42 d     # [v2] mesma semente, Difícil
```

**Controles na janela:**

- **Menu:** `1` `2` `3` ou setas escolhem a dificuldade; `ENTER` começa.
- **Partida:** números = ingredientes, `Z` desfaz, `Espaço` serve.
- **Fim:** `ENTER` joga de novo; `Backspace` volta ao menu.
- **Qualquer tela:** `M` liga ou desliga o som.

Também dá para clicar em tudo.

---

## 3. O mapa do projeto: 3 camadas

O projeto é dividido em **3 camadas**. Cada uma só conversa com a de baixo:

```
┌──────────────────────────────────────────────────────────────┐
│ INTERFACE     main.c + ui.c + som.c [v2]     (usam a raylib) │
│   - lê teclado/mouse, desenha a tela, toca sons,             │
│     chama jogo_tick 1x/s                                     │
└───────────────┬──────────────────────────────▲───────────────┘
   ações        │ jogo_empilhar, jogo_servir,  │  estado da partida
   do jogador   │ jogo_desfazer, jogo_tick     │  (SÓ LEITURA: const)
                │                              │  + Eventos [v2]
┌───────────────▼──────────────────────────────┴───────────────┐
│ REGRAS        jogo.c  (+ pedido.c: cardápio e sorteio)       │
│   - ÚNICO lugar que altera a partida                         │
│   - não sabe o que é raylib, som ou relógio: só conta ticks  │
└───────────────┬──────────────────────────────┬───────────────┘
   enfileirar,  │                              │  empilhar,
   desenfileirar│                              │  desempilhar,
   frente       │                              │  topo
┌───────────────▼────────────┐   ┌─────────────▼───────────────┐
│ ESTRUTURAS  TAD Fila       │   │ TAD Pilha                   │
│ fila.c — vetor circular    │   │ pilha.c — vetor             │
│ elemento: Pedido           │   │ elemento: Ingrediente       │
└────────────────────────────┘   └─────────────────────────────┘
```

**Por que separar assim?**

- **Testar sem janela.** Como `jogo.c` não conhece a raylib, os testes (`tests/`) e o modo texto usam as mesmas regras do jogo real, sem abrir janela.
- **Trocar uma peça sem quebrar as outras.** Na Fase 3, podemos trocar o vetor da fila por uma lista encadeada mexendo **só** em `fila.c`.
- **[v2] Acrescentar sem bagunçar.** A prova disso: para colocar som, o núcleo não precisou saber o que é áudio, só "anotar" o que aconteceu (seção 10).

### Quem inclui quem

```
pedido.h  ←  fila.h  ←┐
          ←  pilha.h ←┼─ jogo.h ← ui.h ← ui.c, main.c
                      │         ← main_texto.c
                      └─ tests/teste_*.c
som.h ← som.c, main.c          [v2] (som.h não depende de ninguém)
```

---

## 4. Os tipos básicos — `pedido.h/.c`

Este módulo não é um TAD de estrutura. Ele define **o que é** cada coisa do jogo.

### Ingrediente (um `enum`)

```c
typedef enum { PAO, CARNE, QUEIJO, ALFACE, TOMATE,   /* 0..4: básicos (Fácil)   */
               BACON, OVO,                           /* 5, 6: [v2] Médio e Difícil */
               CEBOLA                                /* 7:    [v2] só Difícil      */
} Ingrediente;
```

Um `enum` dá **nomes para números**: `PAO` vale 0, `CARNE` vale 1, e assim por diante.

**[v2] A ordem importa.** A dificuldade diz *quantos* ingredientes existem: Fácil = 5 (os 5 primeiros), Médio = 7 e Difícil = 8. Por isso os novos ficaram **no fim** do enum.

### Receita

```c
typedef struct {
    const char *nome;                /* "Cheeseburguer"              */
    Ingrediente itens[RECEITA_MAX];  /* {PAO, CARNE, QUEIJO, PAO}    */
    int tamanho;                     /* 4                            */
} Receita;
```

`itens[0]` é a **base** do lanche, o primeiro a ser empilhado.

### Pedido: o elemento da fila

```c
typedef struct {
    int numero;            /* 1, 2, 3... ordem de chegada */
    Receita receita;
    int paciencia_inicial; /* em ticks                     */
    int valor;             /* pontos se for servido        */
} Pedido;
```

### [v2] RegrasPedido: o que muda com a dificuldade

```c
typedef struct {
    int tamanho_max;                /* maior receita que pode sair  */
    int ingredientes_disponiveis;   /* 5, 7 ou 8                    */
    int paciencia_base;
    int paciencia_por_ingrediente;
    int pontos_por_ingrediente;
} RegrasPedido;
```

O `jogo.c` preenche essa struct a partir da dificuldade e do nível, e a passa por ponteiro **`const`** (só leitura) para:

- `pedido_criar(numero, receita, &regras)`: calcula `paciência = base + por_ingrediente × tamanho` e `valor = pontos × tamanho`;
- `pedido_gerar(numero, &regras, &semente)`: sorteia uma receita que **caiba no tamanho** e **só use ingredientes disponíveis**.

Exemplo no Fácil (base 8, +3 por item, 10 pontos por item):

| Ingredientes | Paciência | Valor |
|---|---|---|
| 3 | 17 ticks | 30 pts |
| 4 | 20 ticks | 40 pts |
| 7 | 29 ticks | 70 pts |

### O cardápio

É um vetor fixo com **19 receitas**: as 10 originais e mais 9 novas na v2. Cada uma tem de 3 a 7 ingredientes. Fica em `pedido.c`. **[v2]** As receitas novas usam Bacon, Ovo ou Cebola e foram colocadas **no fim** do vetor; o motivo está na seção 9.

`receita_disponivel(&r, n)` responde se **todos** os ingredientes da receita estão entre os `n` primeiros do enum.

### O sorteio de pedidos

Usamos uma fórmula simples (gerador congruencial linear), em vez de `rand()`:

```c
*semente = *semente * 1103515245u + 12345u;
return (*semente / 65536u) % 32768u;
```

**Por que não `rand()`?** Porque o `rand()` do Windows é diferente do `rand()` do Linux. Com a nossa fórmula, a **semente 42 gera a mesma partida em qualquer computador**. A semente é passada **por referência** (`unsigned int *semente`): a função muda a variável de quem chamou.

---

## 5. TAD Fila de Pedidos — `fila.h/.c`

*(Sem mudanças na v2.)*

### Por que fila?

A regra "só o cliente da frente pode ser atendido, e quem chega vai para o fim" **é** o comportamento FIFO (*First In, First Out*).

### A interface (`fila.h`)

| Operação | O que faz | Custo |
|---|---|---|
| `Fila *fila_criar(int capacidade)` | cria fila vazia (`NULL` se falhar) | O(1) |
| `void fila_destruir(Fila *f)` | libera a memória | O(1) |
| `bool fila_enfileirar(Fila *f, Pedido p)` | põe `p` no **fim**; `false` se cheia | O(1) |
| `bool fila_desenfileirar(Fila *f, Pedido *removido)` | tira da **frente**; `false` se vazia | O(1) |
| `bool fila_frente(const Fila *f, Pedido *frente)` | olha a frente sem tirar | O(1) |
| `bool fila_vazia(const Fila *f)` / `fila_cheia` | perguntas | O(1) |
| `int fila_tamanho(const Fila *f)` | quantos pedidos | O(1) |
| `bool fila_consultar(const Fila *f, int pos, Pedido *saida)` | **só leitura**, para a tela desenhar | O(1) |

**Estilo da Aula 10:** quem pode falhar devolve `bool` e entrega o resultado por um ponteiro de saída:

```c
Pedido p;
if (fila_desenfileirar(fila, &p)) {
    printf("Saiu o pedido #%d\n", p.numero);
} else {
    printf("A fila estava vazia\n");
}
```

### Tipo opaco: os campos escondidos

No `fila.h` só existe `typedef struct fila Fila;`. Os campos de verdade estão no `fila.c`:

```c
struct fila {
    Pedido *dados;    /* vetor, criado com malloc   */
    int capacidade;   /* tamanho do vetor           */
    int primeiro;     /* índice da FRENTE           */
    int ultimo;       /* próxima posição LIVRE      */
    int quantidade;   /* quantos pedidos há         */
};
```

`jogo.c` **não consegue** escrever `fila->quantidade = 99`, porque o compilador nem sabe que o campo existe. Isso é o **encapsulamento** da Aula 06. Como o tamanho da struct fica escondido, a fila é **criada com `malloc`** e **liberada com `free`** (Aula 09).

**[v2]** A fila é criada com a capacidade da dificuldade: 5 no Fácil e no Médio, 4 no Difícil. A capacidade sempre foi um parâmetro de `fila_criar`, então **o TAD não precisou mudar nada**.

### O vetor circular, passo a passo

Capacidade 5. `p` = primeiro, `u` = último (próxima posição livre).

```
Começo:            [ .  .  .  .  . ]   p=0 u=0 qtd=0   (vazia)
enfileira A, B, C: [ A  B  C  .  . ]   p=0 u=3 qtd=3
desenfileira 2x:   [ .  .  C  .  . ]   p=2 u=3 qtd=1   (A e B saíram)
enfileira D, E:    [ .  .  C  D  E ]   p=2 u=0 qtd=3   ← u deu a VOLTA
enfileira F:       [ F  .  C  D  E ]   p=2 u=1 qtd=4   ← F entrou no índice 0!
```

A "volta" é o operador `%`: `f->ultimo = (f->ultimo + 1) % f->capacidade;`

**Pegadinha clássica:** `primeiro == ultimo` acontece **tanto vazia quanto cheia**. Por isso guardamos `quantidade`.

### `fila_consultar`: isso não "quebra" a fila?

A tela precisa **desenhar todos** os pedidos. `fila_consultar` é **somente leitura** (`const`), usada **só pela interface**. As **regras** nunca a usam: só `enfileirar`, `desenfileirar` e `frente`. A alternativa (desenfileirar tudo e enfileirar de volta a cada quadro) alteraria a fila só para olhar.

```c
*saida = f->dados[(f->primeiro + posicao) % f->capacidade];
```

---

## 6. TAD Pilha de Prato — `pilha.h/.c`

*(Sem mudanças na v2.)*

### Por que pilha?

O lanche é montado **de baixo para cima**, e **desfazer** tira só o **último** colocado. Isso é LIFO (*Last In, First Out*).

### A interface (`pilha.h`)

| Operação | O que faz | Custo |
|---|---|---|
| `Pilha *pilha_criar(int capacidade)` | cria pilha vazia | O(1) |
| `void pilha_destruir(Pilha *p)` | libera a memória | O(1) |
| `bool pilha_empilhar(Pilha *p, Ingrediente ing)` | põe no **topo**; `false` se cheia | O(1) |
| `bool pilha_desempilhar(Pilha *p, Ingrediente *removido)` | tira do **topo**; `false` se vazia | O(1) |
| `bool pilha_topo(const Pilha *p, Ingrediente *topo)` | olha o topo sem tirar | O(1) |
| `pilha_vazia`, `pilha_cheia`, `pilha_tamanho` | perguntas | O(1) |
| `bool pilha_consultar(const Pilha *p, int pos, Ingrediente *saida)` | **só leitura** (0 = base) | O(1) |

### Como funciona (`topo` é um ÍNDICE, como nos slides)

```
  6 [        ]
  5 [        ]
  4 [        ]
  3 [        ]
  2 [ Queijo ]  ← topo = 2
  1 [ Carne  ]
  0 [ Pao    ]  ← base

vazia: topo == -1      cheia: topo == capacidade - 1      tamanho = topo + 1
```

```c
bool pilha_empilhar(Pilha *p, Ingrediente ing) {
    if (pilha_cheia(p)) return false;
    p->topo++;                  /* 1º sobe o topo            */
    p->dados[p->topo] = ing;    /* 2º grava na nova posição  */
    return true;
}
```

---

## 7. O núcleo do jogo — `jogo.h/.c`

É o "cérebro" do jogo. **Só ele altera a partida.**

### A struct `Partida`: todo o estado do jogo

| Campo | Para que serve |
|---|---|
| `Fila *fila` | os clientes esperando (TAD Fila) |
| `Pilha *prato` | o lanche em montagem (TAD Pilha) |
| `Dificuldade dificuldade` | **[v2]** Fácil, Médio ou Difícil |
| `ConfigDificuldade config` | **[v2]** os números dessa dificuldade (seção 9) |
| `int acertos` | contador de acertos (abaixo) |
| `int paciencia_frente` | ticks até o cliente da frente desistir |
| `int tick` | relógio do jogo |
| `int intervalo_chegada` | de quantos em quantos ticks chega cliente |
| `int ticks_para_chegada` | contagem regressiva até o próximo cliente |
| `int nivel` | sobe a cada `config.pedidos_por_nivel` servidos |
| `int pontos, vidas, servidos, perdidos` | placar |
| `int proximo_numero` | número do próximo cliente |
| `unsigned int semente` | estado do sorteio |
| `bool terminou` | acabaram as vidas? |
| `char mensagem[96]` | o aviso que aparece na tela |
| `Eventos eventos` | **[v2]** o que aconteceu (para os sons, seção 10) |

A `Partida` **não** é opaca (a tela precisa ler pontos, vidas...). A proteção vem do **`const`**: a interface recebe `const Partida *` e, se tentar alterar algo, o **compilador dá erro**.

### A ideia mais importante: o CONTADOR DE ACERTOS

**Problema:** conferir o prato percorrendo a pilha descaracteriza o TAD (a pilha só mostra o topo).

**Solução:** guardar `acertos` = **quantos ingredientes, contando da base, estão certos e sem erro no meio**.

Exemplo com a receita **Pão, Carne, Queijo, Pão**:

| Ação | Prato (base → topo) | acertos | Por quê |
|---|---|---|---|
| empilha Pão | Pão | **1** | Pão = receita[0] ✔ |
| empilha Carne | Pão, Carne | **2** | Carne = receita[1] ✔ |
| empilha Alface | Pão, Carne, Alface | **2** | Alface ≠ receita[2] ✘ |
| empilha Queijo | Pão, Carne, Alface, Queijo | **2** | já há erro embaixo: nada mais conta |
| desfaz (sai Queijo) | Pão, Carne, Alface | **2** | removido não era acerto |
| desfaz (sai Alface) | Pão, Carne | **2** | removido não era acerto |
| empilha Queijo | Pão, Carne, Queijo | **3** | Queijo = receita[2] ✔ |
| empilha Pão | Pão, Carne, Queijo, Pão | **4** | → pode servir! |

**As 3 regras** (cada uma O(1)):

1. **Empilhar `x`:** se `acertos == tamanho antes` e `receita[acertos] == x`, então `acertos++`.
2. **Desfazer:** se `acertos == tamanho antes`, o que saiu era acerto, então `acertos--`.
3. **Servir é válido** quando `acertos == tamanho do prato == tamanho da receita`.

Na tela, é a **bolinha verde ou vermelha** ao lado de cada camada. Quando a frente muda (servido ou desistiu), **o prato é descartado**.

### `jogo_tick`: o que acontece a cada segundo

```
jogo_tick(p):
  se a partida terminou → não faz nada
  tick++

  1) PACIÊNCIA (só do cliente da frente)
     se há alguém na fila:
        paciencia_frente--
        se chegou a 0:
           desenfileira (sai pela FRENTE), descarta o prato, perde 1 vida
           paciencia_frente = paciência cheia do NOVO cliente da frente

  2) CHEGADA DE CLIENTES
     ticks_para_chegada--
     se chegou a 0:
        sorteia um pedido novo (com as regras da dificuldade e do nível)
        se a fila está no limite (config.max_pedidos_fila) → ele vai embora: -1 vida
        senão → enfileira no FIM
        ticks_para_chegada = intervalo_chegada
```

**Por que só a frente perde paciência?** Senão alguém do **meio** poderia desistir, e tirar do meio **não é operação de fila**.

**Por que o limite da fila é conferido no `jogo.c`?** É **regra do jogo**, não detalhe da estrutura. Se a fila virar encadeada, o jogo continua igual.

### `jogo_servir`

```
se não há cliente → falha
se acertos != tamanho do prato → falha ("Prato errado!")      ┐ [v2] no Médio/Difícil:
se prato menor que a receita → falha ("Ainda faltam N...")    ┘ cliente perde paciência
senão (prato perfeito):
   desenfileira o cliente, descarta o prato, soma os pontos
   servidos++ → a cada config.pedidos_por_nivel: nível++, intervalo-- (até o mínimo)
   paciencia_frente = a do novo cliente da frente
```

### `jogo_empilhar` e `jogo_desfazer`

- **Empilhar** falha se:
  - não há cliente;
  - o prato já tem 7;
  - **[v2]** o ingrediente não existe na dificuldade (ex.: Bacon no Fácil).

  Se não falhar, faz `pilha_empilhar` e atualiza `acertos`.
- **Desfazer:** faz `pilha_desempilhar` (só o topo!) e atualiza `acertos`. Falha se o prato está vazio.

Toda função que falha **não muda o prato nem a fila** e explica o motivo em `p->mensagem`.

### Funções `static`

`descartar_prato`, `perder_vida`, `chegar_cliente`, `aplicar_penalidade` **[v2]** etc. são `static`: ajudantes que só existem dentro de `jogo.c`. `descartar_prato` esvazia o prato **usando só `pilha_desempilhar`**:

```c
while (pilha_desempilhar(p->prato, NULL)) { }
```

---

## 8. A interface — `ui.c` e `main.c`

Junto com `som.c`, são os **únicos** arquivos que usam a raylib.

### O laço principal (`main.c`)

Um jogo é um laço que roda cerca de 60 vezes por segundo. Cada volta é um **quadro** (*frame*):

```c
while (!WindowShouldClose()) {
    Acao acao = ui_ler_acao(tela, &partida, escolhida);  // 1) o que o jogador fez?
    aplicar_acao(&partida, acao);                        // 2) chama o núcleo
    acumulado += GetFrameTime();                         // 3) relógio
    while (acumulado >= 1.0f) { jogo_tick(&partida); acumulado -= 1.0f; }
    tocar_eventos(&partida.eventos);                     // 4) [v2] sons
    jogo_limpar_eventos(&partida);
    BeginDrawing();
    ui_desenhar(tela, &partida, escolhida, som_esta_ligado());  // 5) desenha
    EndDrawing();
}
```

(O código de verdade também troca de tela: **menu → jogo → fim → jogo ou menu**.)

**O relógio:** `GetFrameTime()` devolve quanto durou o último quadro (cerca de 0,016 s). Somamos; a cada 1 segundo, um `jogo_tick`. **O núcleo não sabe o que é "segundo".**

### Acao: separando "ler" de "fazer"

`ui_ler_acao` só **descobre** o que o jogador quer:

```c
typedef struct { TipoAcao tipo; Ingrediente ingrediente; Dificuldade dificuldade; } Acao;
```

Os tipos são: `EMPILHAR`, `DESFAZER`, `SERVIR`, `CONFIRMAR` e, na **[v2]**, `ESCOLHER_DIFICULDADE`, `VOLTAR_MENU` e `ALTERNAR_SOM`. Quem **executa** é o `main.c`. Assim a interface nunca altera o jogo.

### As telas (`ui_desenhar`, que só lê)

- **[v2] Menu:** instruções + **3 cartões de dificuldade** (os textos vêm de `jogo_config(d)`) + botão "Comecar".
- **Jogo:**
  - **cabeçalho:** pontos, servidos, nível, vidas e **[v2]** a etiqueta da dificuldade;
  - **fila (esquerda):** `fila_consultar` em cada posição; a frente tem borda laranja e barra de paciência; **[v2]** 5 ou 4 espaços;
  - **prato (meio):** `pilha_consultar` em cada camada; bolinha verde ou vermelha vinda do `acertos`;
  - **receita da frente (direita):** "ok" nos acertos e destaque laranja no próximo;
  - **botões:** **[v2]** 5, 7 ou 8 ingredientes, com a largura dividida automaticamente (`retangulo_ingrediente(i, n)`), mais Desfazer e Servir;
  - **[v2]** indicador "Som: ligado (M)".
- **Fim:** placar por cima da partida escurecida, com **[v2]** a dificuldade e a opção "BACKSPACE = trocar dificuldade".

As funções `retangulo_...()` são usadas **tanto para desenhar quanto para detectar o clique**, então nunca ficam desalinhadas.

### Modo `--captura`

`./fast-grill.exe --captura` joga sozinho e salva 5 imagens. `make capturas` faz isso e move as imagens para `docs/`. As imagens são `captura_menu`, `captura_jogo`, `captura_erro`, `captura_fim` e **[v2]** `captura_dificil`.

---

## 9. [v2] As dificuldades

### Onde ficam os números

Em **uma tabela só**, no começo do `jogo.c`:

```c
static const ConfigDificuldade CONFIGS[NUM_DIFICULDADES] = {
    /*  nome      ingr pac.base pac/ingr pts/ingr interv.ini interv.min tam.ini ped/nivel fila penal */
    { "Facil",     5,    8,       3,      10,       10,        4,         4,      5,       5,    0 },
    { "Medio",     7,    6,       2,      15,        8,        3,         5,      4,       5,    2 },
    { "Dificil",   8,    5,       2,      20,        6,        3,         6,      3,       4,    3 }
};
```

`jogo_iniciar(&p, semente, dificuldade)` copia a linha escolhida para `p->config`, e o resto do núcleo usa `p->config.xxx` onde antes havia um `#define`. Ou seja: **as regras são as mesmas, só os números mudam.**

| | Fácil | Médio | Difícil |
|---|---|---|---|
| Ingredientes | 5 | 7 (+Bacon, Ovo) | 8 (+Cebola) |
| Paciência (receita de 4) | 20 ticks | 14 ticks | 13 ticks |
| Novo cliente a cada | 10 s (→ 4) | 8 s (→ 3) | 6 s (→ 3) |
| Receita máxima no nível 1 | 4 | 5 | 6 |
| Servidos por nível | 5 | 4 | 3 |
| Fila lota com | 5 | 5 | 4 |
| Servir errado | — | −2 paciência | −3 paciência |
| Pontos por item | 10 | 15 | 20 |

### O Fácil é exatamente a versão 1

Três cuidados garantem isso:

1. A linha "Facil" tem os mesmos números dos antigos `#define`.
2. As receitas novas estão **no fim** do cardápio. No Fácil elas são filtradas, então a lista de candidatas no sorteio é a mesma de antes.
3. Penalidade 0.

**Verificado:** a mesma partida (semente 42) dá a mesma saída, tick por tick, na v1 e na v2.

### A penalidade por servir errado

```c
static void aplicar_penalidade(Partida *p) {
    if (p->config.penalidade_erro <= 0) return;      /* Fácil: nada */
    p->paciencia_frente -= p->config.penalidade_erro;
    if (p->paciencia_frente < 1) p->paciencia_frente = 1;
}
```

**Por que para em 1, e não deixa zerar?** Porque a desistência do cliente (tirar da fila, descartar o prato, perder vida) acontece **num lugar só**: o `jogo_tick`. Se a penalidade também fizesse isso, teríamos a mesma regra escrita em dois lugares, e um dia elas iam ficar diferentes.

---

## 10. [v2] Os efeitos sonoros — `som.h/.c`

### A ideia: o núcleo anota, o `main.c` toca

O `jogo.c` **não pode** conhecer som (ele é testado sem janela e sem áudio). Então:

1. O núcleo tem uma struct `Eventos` dentro da `Partida`:

   ```c
   typedef struct {
       bool empilhou, desfez, serviu, errou,
            cliente_chegou, perdeu_vida, subiu_nivel, terminou;
   } Eventos;
   ```

   Quando algo acontece, ele só marca: `p->eventos.serviu = true;`
2. A cada quadro, o `main.c` chama `tocar_eventos(&partida.eventos)`, que toca o som de cada evento marcado, e depois `jogo_limpar_eventos(&partida)`.
3. O `som.c` sabe **tocar**: `som_tocar(SOM_SERVIR)`.

Resultado: dá para **testar** que os eventos são marcados (os testes da seção `[v2] Eventos`) sem precisar ouvir nada.

### Como os sons são gerados (sem arquivos de áudio)

Som digital é uma **lista de números (amostras)**: 44100 por segundo, cada um um `short` de −32767 a 32767, dizendo a posição do alto-falante. Uma **nota** é uma onda que se repete *frequência* vezes por segundo (Lá = 440 Hz; quanto maior, mais agudo).

Para cada amostra `i`:

```
tempo    = i / 44100
fase     = parte fracionária de (frequência × tempo)      → 0.0 a 1.0
valor    = onda(fase) × envelope × volume × 32767
```

- **Formas de onda:** **seno** (suave, campainha), **quadrada** (som de videogame antigo) e **triângulo** (meio-termo).
- **Envelope:** o volume sobe rápido no começo (5 ms) e desce até 0 no fim da nota. Sem isso, cada nota faria um "estalo".

A função `criar_som(notas, num_notas, duracao, forma, volume)`:

1. aloca o vetor de amostras com `malloc` (Aula 09);
2. preenche o vetor com a conta acima;
3. entrega para a raylib (`LoadSoundFromWave`, que copia os dados);
4. dá `free`.

### Os 10 sons

| Evento | Som | Onda |
|---|---|---|
| empilhou | "pop" (Lá5) | triângulo |
| desfez | "pop" descendo | triângulo |
| serviu | arpejo Dó-Mi-Sol-Dó subindo | quadrada |
| errou | zumbido grave | quadrada |
| cliente chegou | campainha "plim-plom" | seno |
| perdeu vida | três notas descendo | triângulo |
| subiu de nível | arpejo rápido e agudo | seno |
| terminou | notas longas descendo | triângulo |
| menu / M | clique | seno |
| partida começou | duas notas | quadrada |

Se o PC não tiver saída de áudio, `som_iniciar` percebe (`IsAudioDeviceReady`) e o jogo segue mudo, sem erro. **Tecla `M`** liga ou desliga.

---

## 11. O modo texto — `main_texto.c`

O **mesmo jogo** no terminal, sem raylib e sem som. O tempo **não anda sozinho**: digite `t` para passar um tick.

```
./fast-grill-texto.exe 42            # semente 42, Fácil
./fast-grill-texto.exe 42 medio      # [v2] dificuldade: facil | medio | dificil (basta a 1a letra)

> 1          (empilha Pão)
> 3          (empilha Queijo)
> s          (serve)
> t 5        (passa 5 ticks)
```

Também aceita comandos por arquivo. Assim foram gerados `docs/demo_texto.txt` (Fácil) e **[v2]** `docs/demo_texto_dificil.txt`.

---

## 12. Os testes — `tests/`

### Como funcionam

`verifica(condicao, "descricao")` imprime `[ OK ]` ou `[FALHOU]`. No fim, `resumo_testes` devolve 0 (tudo certo) ou 1. O `make test` para com erro se algo falhar.

### O que cada arquivo testa

| Arquivo | Testes | O que confere |
|---|---|---|
| `teste_fila.c` | 37 | vazia/cheia, FIFO, **volta do índice circular**, consultar, 100 entradas e saídas |
| `teste_pilha.c` | 31 | vazia/cheia, LIFO, topo, consultar, reuso |
| `teste_jogo.c` | 119 | cardápio, início, servir certo/errado/incompleto, **contador de acertos**, desfazer, desistência, fila lotada, progressão, fim + **[v2]** ingredientes por dificuldade, tabela de dificuldades, penalidade, fila de 4 no Difícil e eventos de som |

**Total: 187 testes** (eram 143 na v1). Os testes antigos de regras rodam no **Fácil**.

### Os testes pegam erros de verdade?

Sim. Foram colocados erros **de propósito** em cópias do `jogo.c`:

- `acertos` diminuindo sempre no desfazer;
- **[v2]** a penalidade sem o "piso" de 1;
- **[v2]** o intervalo do Fácil mudado de 10 para 9.

Em cada caso, o teste certo falhou na hora.

---

## 13. Uma tecla, do começo ao fim

O que acontece quando o jogador aperta **`2` (Carne)** na janela:

```
1. main.c   → ui_ler_acao(TELA_JOGO, &partida, ...)
2. ui.c     → IsKeyPressed(KEY_TWO) → devolve { ACAO_EMPILHAR, CARNE }
3. main.c   → aplicar_acao → jogo_empilhar(&partida, CARNE)
4. jogo.c   → CARNE existe nesta dificuldade? sim [v2]
              fila_frente(...)      : receita do cliente da frente
              pilha_tamanho(...)    : prato tinha 1 item
              pilha_empilhar(...)   : Carne vai para o TOPO
              acertos == 1 e receita[1] == CARNE → acertos = 2
              eventos.empilhou = true [v2] ; mensagem = "Carne no prato."
5. pilha.c  → topo++ ; dados[topo] = CARNE
6. main.c   → tocar_eventos → som_tocar(SOM_EMPILHAR) → "pop!" [v2]
              jogo_limpar_eventos
7. main.c   → ui_desenhar(TELA_JOGO, &partida, ...)   (const: só lê)
8. ui.c     → pilha_consultar(prato, 0..1) desenha Pão e Carne com bolinhas verdes
```

---

## 14. Perguntas que o professor pode fazer

**Por que fila para os pedidos?**
"Só atende a frente, e quem chega vai para o fim" é FIFO. As operações são O(1).

**Por que pilha para o prato?**
O lanche é montado de baixo para cima e "desfazer" tira só o último. Isso é LIFO, com operações O(1).

**Por que fila circular?**
Para reaproveitar o espaço liberado no início do vetor sem deslocar elementos (deslocar seria O(n)).

**Como sabem se a fila está vazia ou cheia?**
Pelo campo `quantidade`; `primeiro == ultimo` acontece nos dois casos.

**Como conferem o prato sem percorrer a pilha?**
Com o contador de acertos, atualizado em O(1) (seção 7).

**`consultar` não quebra o TAD?**
É somente leitura (`const`), usada só pela interface. As regras nunca a usam.

**Por que tipo opaco?**
Encapsulamento, e dá para trocar vetor por lista encadeada sem mudar o resto.

**Por que só a frente perde paciência?**
Para nunca remover alguém do meio da fila.

**[v2] Como a dificuldade foi implementada sem mexer nos TADs?**
Os TADs já recebiam a capacidade como parâmetro (`fila_criar(capacidade)`). A dificuldade só muda **números** (tabela `CONFIGS`) e quais receitas podem ser sorteadas. As regras e as estruturas são as mesmas.

**[v2] Por que a penalidade não faz o cliente desistir na hora?**
Para a desistência continuar num único lugar (`jogo_tick`) e não duplicar regra.

**[v2] Como o núcleo toca som se ele não conhece a raylib?**
Ele não toca: só **anota eventos** na `Partida`. Quem toca é o `main.c`. Assim o núcleo continua testável sem janela e sem áudio.

**[v2] De onde vêm os sons?**
São gerados pelo nosso código (`som.c`): ondas calculadas amostra por amostra. Não usamos arquivos de áudio de terceiros.

**Onde está a raylib? O que é de vocês?**
A raylib só aparece em `ui.c`, `main.c` e `som.c`. Fila, pilha, regras, dificuldades, testes e modo texto não usam nada externo.

---

## 15. Como mexer no código

### Mudar a dificuldade ou o balanceamento [v2]

Editem a tabela **`CONFIGS[]`** no começo do `src/jogo.c`. O menu se atualiza sozinho. Depois rodem `make test`: os testes conferem que o Fácil continua igual à v1 e que nenhuma regra fica **mais fácil** do Fácil para o Médio ou do Médio para o Difícil.

### Adicionar uma receita

No `CARDAPIO` de `src/pedido.c`, **no fim**, para não mudar as partidas do Fácil:

```c
{ "X-Picanha", { PAO, CARNE, CEBOLA, QUEIJO, PAO }, 5 },
```

Ela só aparece nas dificuldades que têm todos os ingredientes dela. Esta tem Cebola, então só no Difícil.

### Mudar um som [v2]

Em `som_iniciar()` (`src/som.c`): troque as notas, a duração, a forma da onda ou o volume da linha do som.

### Adicionar um teste

Em `tests/teste_jogo.c`, crie `static void testar_algo(void)` com chamadas `verifica(...)` e chame no `main`.

### Depurar

O Makefile compila com `-g`: `gdb ./tests/teste_jogo.exe` (o gdb vem no w64devkit).

---

## 16. O que fica para a Fase 3

- **Fila e pilha encadeadas** (conteúdo de 23/11), **sem mudar `fila.h` e `pilha.h`**. Todos os 187 testes devem continuar passando.
- **Balanceamento** das 3 dificuldades com partidas reais do grupo.
- Extensão opcional: cliente VIP com **fila de prioridade** (conteúdo de 30/11).

---

## 17. Glossário

| Termo | Significado |
|---|---|
| **TAD** | Tipo Abstrato de Dados: dados + operações, usados como "caixa-preta" |
| **FIFO** | *First In, First Out*: fila |
| **LIFO** | *Last In, First Out*: pilha |
| **Tipo opaco** | struct declarada no `.h` sem os campos; só o `.c` os conhece |
| **include guard** | `#ifndef X_H / #define X_H / #endif`: evita incluir o `.h` duas vezes |
| **`malloc` / `free`** | reserva e libera memória durante a execução |
| **`const`** | "não pode ser alterado por aqui"; o compilador garante |
| **`static` (função)** | função visível só dentro do próprio `.c` |
| **`enum`** | lista de nomes para números (`PAO` = 0, `CARNE` = 1...) |
| **tick** | uma "batida" do relógio do jogo (1 por segundo na janela) |
| **quadro / frame** | uma volta do laço principal (cerca de 60 por segundo) |
| **semente** | número inicial do sorteio; mesma semente = mesma sequência |
| **O(1)** | custo constante, não depende do tamanho da estrutura |
| **raylib** | biblioteca externa para janela, desenho, teclado/mouse e áudio |
| **[v2] configuração (`ConfigDificuldade`)** | os números de uma dificuldade, em uma struct |
| **[v2] evento** | anotação do núcleo de que algo aconteceu (usada para tocar som) |
| **[v2] amostra** | um número da lista que forma o som (44100 por segundo) |
| **[v2] frequência** | quantas vezes a onda se repete por segundo (Hz); maior = mais agudo |
| **[v2] envelope** | variação do volume dentro da nota (evita estalos) |
