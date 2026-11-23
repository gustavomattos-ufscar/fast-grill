/*
 * pedido.c — Implementação do módulo de tipos básicos (ver pedido.h).
 *
 * Contém:
 *   - os nomes dos ingredientes;
 *   - o CARDÁPIO fixo (vetor de receitas);
 *   - a criação e o sorteio de pedidos.
 *
 * [v2] Novos ingredientes (Bacon, Ovo, Cebola), novas receitas e regras
 * de pedido por dificuldade (struct RegrasPedido).
 */

#include <stddef.h>   /* NULL */
#include "pedido.h"

/* ------------------------------------------------------------------------
 * Nomes dos ingredientes.
 * O vetor é indexado pelo próprio enum: NOMES[PAO] = "Pao", NOMES[CARNE] =
 * "Carne"... Por isso a ordem aqui TEM que ser a mesma do enum em pedido.h.
 * 'static' = visível só dentro deste arquivo (ninguém de fora acessa).
 * ------------------------------------------------------------------------ */
static const char *NOMES[NUM_INGREDIENTES] = {
    "Pao", "Carne", "Queijo", "Alface", "Tomate",
    "Bacon", "Ovo", "Cebola"                       /* [v2] novos */
};

/* ------------------------------------------------------------------------
 * Cardápio fixo do jogo.
 * Cada linha é uma Receita: { nome, {itens da base para o topo}, tamanho }.
 * Itens não usados do vetor ficam com 0 (o C completa com zeros), mas
 * nunca são lidos porque o 'tamanho' limita o acesso.
 *
 * [v2] As receitas novas foram colocadas NO FIM do vetor. Motivo: no
 * Fácil elas são filtradas (usam Bacon/Ovo/Cebola), então a lista de
 * candidatas do Fácil fica exatamente igual à da versão 1 e a mesma
 * semente gera exatamente a mesma partida de antes.
 * ------------------------------------------------------------------------ */
static const Receita CARDAPIO[] = {
    /* --- receitas da versão 1 (só ingredientes básicos) --- */
    /* 3 ingredientes */
    { "Hamburguer",    { PAO, CARNE, PAO },                                 3 },
    { "Pao com Queijo",{ PAO, QUEIJO, PAO },                                3 },
    /* 4 ingredientes */
    { "Cheeseburguer", { PAO, CARNE, QUEIJO, PAO },                         4 },
    { "X-Tomate",      { PAO, CARNE, TOMATE, PAO },                         4 },
    { "Natural",       { PAO, ALFACE, TOMATE, PAO },                        4 },
    /* 5 ingredientes */
    { "X-Salada",      { PAO, CARNE, QUEIJO, ALFACE, PAO },                 5 },
    { "Vegetariano",   { PAO, QUEIJO, ALFACE, TOMATE, PAO },                5 },
    /* 6 ingredientes */
    { "X-Tudo",        { PAO, CARNE, QUEIJO, ALFACE, TOMATE, PAO },         6 },
    { "Duplo",         { PAO, CARNE, QUEIJO, CARNE, QUEIJO, PAO },          6 },
    /* 7 ingredientes */
    { "Monstro",       { PAO, CARNE, QUEIJO, TOMATE, CARNE, ALFACE, PAO },  7 },

    /* --- [v2] receitas com Bacon e Ovo (Médio e Difícil) --- */
    { "Pao com Ovo",   { PAO, OVO, PAO },                                   3 },
    { "X-Bacon",       { PAO, CARNE, BACON, PAO },                          4 },
    { "X-Egg",         { PAO, CARNE, OVO, QUEIJO, PAO },                    5 },
    { "Bacon Salada",  { PAO, BACON, ALFACE, TOMATE, PAO },                 5 },
    { "X-Bacon Egg",   { PAO, CARNE, BACON, OVO, QUEIJO, PAO },             6 },
    { "Completo",      { PAO, CARNE, BACON, OVO, QUEIJO, ALFACE, PAO },     7 },

    /* --- [v2] receitas com Cebola (só Difícil) --- */
    { "Cebolada",      { PAO, CARNE, CEBOLA, PAO },                         4 },
    { "Smash Cebola",  { PAO, CARNE, CEBOLA, CARNE, QUEIJO, PAO },          6 },
    { "Supremo",       { PAO, CARNE, CEBOLA, BACON, OVO, TOMATE, PAO },     7 }
};

/* Quantidade de receitas = tamanho total do vetor / tamanho de uma receita.
 * Truque com sizeof (Aula 08): se alguém adicionar uma receita acima, este
 * número se atualiza sozinho. */
#define NUM_RECEITAS ((int)(sizeof(CARDAPIO) / sizeof(CARDAPIO[0])))

/* ------------------------------------------------------------------------ */

const char *ingrediente_nome(Ingrediente ing) {
    /* Protege contra valores fora do enum (ex.: um int qualquer convertido). */
    if ((int)ing < 0 || (int)ing >= NUM_INGREDIENTES) {
        return "?";
    }
    return NOMES[ing];
}

int cardapio_quantidade(void) {
    return NUM_RECEITAS;
}

bool cardapio_receita(int indice, Receita *saida) {
    if (saida == NULL || indice < 0 || indice >= NUM_RECEITAS) {
        return false;          /* índice inválido: não mexe em *saida */
    }
    *saida = CARDAPIO[indice]; /* copia a struct inteira */
    return true;
}

/* [v2] Confere ingrediente por ingrediente: basta UM fora da faixa
 * disponível para a receita ficar indisponível. */
bool receita_disponivel(const Receita *r, int ingredientes_disponiveis) {
    int i;
    for (i = 0; i < r->tamanho; i++) {
        if ((int)r->itens[i] >= ingredientes_disponiveis) {
            return false;
        }
    }
    return true;
}

/* [v2] Paciência e valor agora vêm das regras da dificuldade.
 * No Fácil: paciência = 8 + 3 x tamanho e valor = 10 x tamanho
 * (exatamente os valores fixos da versão 1). */
Pedido pedido_criar(int numero, Receita receita, const RegrasPedido *regras) {
    Pedido p;
    p.numero = numero;
    p.receita = receita;
    /* Receitas maiores dão mais tempo e mais pontos. */
    p.paciencia_inicial = regras->paciencia_base
                        + regras->paciencia_por_ingrediente * receita.tamanho;
    p.valor = regras->pontos_por_ingrediente * receita.tamanho;
    return p;
}

/* ------------------------------------------------------------------------
 * Gerador de números pseudoaleatórios (Gerador Congruencial Linear).
 *
 * É a fórmula do exemplo de rand() que aparece no próprio padrão da
 * linguagem C:  semente = semente * 1103515245 + 12345
 * Como 'semente' é unsigned, quando a conta passa do limite ela "dá a
 * volta" (resto da divisão por 2^32), e isso é o comportamento esperado.
 *
 * Por que não usar rand() direto? Porque rand() muda de um sistema para
 * outro (Windows x Linux). Com a nossa fórmula, a semente 42 gera a mesma
 * partida em qualquer computador do grupo — ótimo para testes e para a
 * demonstração.
 * ------------------------------------------------------------------------ */
int sortear(unsigned int *semente) {
    *semente = *semente * 1103515245u + 12345u;
    return (int)((*semente / 65536u) % 32768u);
}

/* [v2] Uma receita é CANDIDATA se cabe no tamanho máximo E só usa
 * ingredientes disponíveis na dificuldade. (Antes era só o tamanho.) */
static bool eh_candidata(const Receita *r, const RegrasPedido *regras, int tamanho_max) {
    return r->tamanho <= tamanho_max &&
           receita_disponivel(r, regras->ingredientes_disponiveis);
}

Pedido pedido_gerar(int numero, const RegrasPedido *regras, unsigned int *semente) {
    int i;
    int candidatas = 0;
    int tamanho_max = regras->tamanho_max;

    /* Garante que o limite esteja dentro do que o cardápio tem. */
    if (tamanho_max < RECEITA_MIN) tamanho_max = RECEITA_MIN;
    if (tamanho_max > RECEITA_MAX) tamanho_max = RECEITA_MAX;

    /* 1º passo: conta quantas receitas são candidatas. */
    for (i = 0; i < NUM_RECEITAS; i++) {
        if (eh_candidata(&CARDAPIO[i], regras, tamanho_max)) {
            candidatas++;
        }
    }

    /* Segurança: sempre existe ao menos o Hamburguer (3 itens básicos),
     * mas se alguém mexer no cardápio e não sobrar candidata, usamos ele. */
    if (candidatas == 0) {
        return pedido_criar(numero, CARDAPIO[0], regras);
    }

    /* 2º passo: sorteia QUAL das candidatas será usada (0..candidatas-1). */
    int escolhida = sortear(semente) % candidatas;

    /* 3º passo: percorre de novo até achar a candidata de número 'escolhida'. */
    for (i = 0; i < NUM_RECEITAS; i++) {
        if (eh_candidata(&CARDAPIO[i], regras, tamanho_max)) {
            if (escolhida == 0) {
                return pedido_criar(numero, CARDAPIO[i], regras);
            }
            escolhida--;
        }
    }

    /* Nunca chega aqui, mas o compilador exige um return em todos os caminhos. */
    return pedido_criar(numero, CARDAPIO[0], regras);
}
