/*
 * pedido.h — Tipos básicos do jogo: Ingrediente, Receita e Pedido.
 *
 * Este módulo NÃO é um TAD de estrutura de dados. Ele só define "o que é"
 * um ingrediente, uma receita e um pedido, e guarda o cardápio fixo do
 * jogo. Os TADs Fila (fila.h) e Pilha (pilha.h) usam estes tipos como
 * elemento: a fila guarda Pedidos e a pilha guarda Ingredientes.
 *
 * Por que separar em um módulo próprio?
 *   - fila.h e pilha.h precisam conhecer o tipo do elemento;
 *   - jogo.c precisa sortear pedidos do cardápio;
 *   - ui.c precisa saber o nome de cada ingrediente para desenhar.
 * Colocando tudo aqui, ninguém depende de quem não precisa.
 *
 * [v2] MUDANÇAS DA VERSÃO 2 (dificuldade):
 *   - 3 ingredientes novos: BACON, OVO e CEBOLA (só aparecem no Médio e
 *     no Difícil);
 *   - novas receitas no cardápio que usam esses ingredientes;
 *   - as constantes de paciência e pontos (PACIENCIA_BASE...) saíram
 *     daqui: agora elas dependem da dificuldade e chegam pela struct
 *     RegrasPedido (preenchida pelo jogo.c).
 */

#ifndef PEDIDO_H   /* include guard: evita incluir este arquivo duas vezes */
#define PEDIDO_H

#include <stdbool.h>

/* ------------------------------------------------------------------------
 * Constantes das receitas
 * ------------------------------------------------------------------------ */
#define RECEITA_MIN 3   /* menor receita do cardápio (ex.: pão, carne, pão) */
#define RECEITA_MAX 7   /* maior receita; é também a capacidade do prato   */

/* ------------------------------------------------------------------------
 * Ingrediente
 *
 * Um enum dá um nome para cada número: PAO vale 0, CARNE vale 1, etc.
 * Assim o código fica legível (PAO em vez de 0) e o compilador avisa se
 * usarmos um valor que não existe.
 *
 * [v2] A ORDEM IMPORTA: os 5 primeiros são os ingredientes "básicos" (os
 * únicos do Fácil). A dificuldade diz QUANTOS ingredientes estão
 * disponíveis: Fácil = 5 (PAO..TOMATE), Médio = 7 (+BACON, OVO),
 * Difícil = 8 (+CEBOLA). Por isso os novos ficam no FIM do enum.
 * ------------------------------------------------------------------------ */
typedef enum {
    PAO,       /* 0 */
    CARNE,     /* 1 */
    QUEIJO,    /* 2 */
    ALFACE,    /* 3 */
    TOMATE,    /* 4 */
    BACON,     /* 5  [v2] novo: Médio e Difícil */
    OVO,       /* 6  [v2] novo: Médio e Difícil */
    CEBOLA     /* 7  [v2] novo: só Difícil      */
} Ingrediente;

#define NUM_INGREDIENTES 8          /* [v2] era 5: total de valores do enum */
#define NUM_INGREDIENTES_BASICOS 5  /* [v2] PAO..TOMATE (os do Fácil)       */

/* ------------------------------------------------------------------------
 * Receita: o lanche que o cliente quer, NA ORDEM em que deve ser montado.
 *
 * itens[0] é a BASE do lanche (o primeiro a ser empilhado no prato) e
 * itens[tamanho - 1] é o TOPO (o último a ser empilhado).
 * Exemplo "Cheeseburguer": {PAO, CARNE, QUEIJO, PAO}, tamanho = 4.
 * ------------------------------------------------------------------------ */
typedef struct {
    const char *nome;                /* nome exibido na tela                 */
    Ingrediente itens[RECEITA_MAX];  /* só as posições 0..tamanho-1 valem   */
    int tamanho;                     /* entre RECEITA_MIN e RECEITA_MAX      */
} Receita;

/* ------------------------------------------------------------------------
 * Pedido: um cliente na fila. É o ELEMENTO guardado no TAD Fila.
 * ------------------------------------------------------------------------ */
typedef struct {
    int numero;             /* 1, 2, 3...: ordem de chegada do cliente     */
    Receita receita;        /* o que deve ser montado                      */
    int paciencia_inicial;  /* quantos ticks o cliente espera na frente    */
    int valor;              /* pontos ganhos ao servir este pedido         */
} Pedido;

/* ------------------------------------------------------------------------
 * [v2] RegrasPedido: as regras que mudam com a dificuldade e afetam a
 * criação de um pedido. O jogo.c preenche esta struct a partir da
 * dificuldade escolhida e do nível atual, e a passa por ponteiro const
 * (só leitura) para pedido_criar e pedido_gerar.
 * ------------------------------------------------------------------------ */
typedef struct {
    int tamanho_max;                /* maior receita que pode ser sorteada  */
    int ingredientes_disponiveis;   /* 5, 7 ou 8 (ver enum acima)           */
    int paciencia_base;             /* paciência = base + por_ingrediente x */
    int paciencia_por_ingrediente;  /*             tamanho da receita       */
    int pontos_por_ingrediente;     /* valor = pontos x tamanho da receita  */
} RegrasPedido;

/* ------------------------------------------------------------------------
 * Operações do módulo
 * ------------------------------------------------------------------------ */

/* Devolve o nome de um ingrediente (ex.: CARNE -> "Carne").
 * Os nomes não têm acento porque a fonte padrão da raylib não desenha
 * caracteres acentuados. */
const char *ingrediente_nome(Ingrediente ing);

/* Quantas receitas existem no cardápio fixo. */
int cardapio_quantidade(void);

/* Copia para *saida a receita da posição 'indice' do cardápio.
 * Retorna false (e não mexe em *saida) se o índice for inválido. */
bool cardapio_receita(int indice, Receita *saida);

/* [v2] true se TODOS os ingredientes da receita estão entre os
 * 'ingredientes_disponiveis' primeiros do enum (ex.: com 5, uma receita
 * que tem BACON não está disponível). */
bool receita_disponivel(const Receita *r, int ingredientes_disponiveis);

/* Monta um Pedido a partir de uma receita: calcula a paciência e o valor
 * a partir do tamanho da receita e das regras da dificuldade.
 * [v2] ganhou o parâmetro 'regras' (antes usava constantes fixas). */
Pedido pedido_criar(int numero, Receita receita, const RegrasPedido *regras);

/* Sorteia uma receita do cardápio que respeite as regras (tamanho máximo
 * e ingredientes disponíveis) e devolve o Pedido correspondente.
 * 'semente' é o estado do gerador de números pseudoaleatórios: a função
 * atualiza *semente a cada sorteio (passagem por referência). A mesma
 * semente inicial sempre gera a MESMA sequência de pedidos, o que deixa
 * os testes reproduzíveis.
 * [v2] recebe 'regras' no lugar do antigo 'tamanho_max'. */
Pedido pedido_gerar(int numero, const RegrasPedido *regras, unsigned int *semente);

/* Gerador de números pseudoaleatórios (devolve um valor entre 0 e 32767).
 * Atualiza *semente. Usado por pedido_gerar. */
int sortear(unsigned int *semente);

#endif /* PEDIDO_H */
