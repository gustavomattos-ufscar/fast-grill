/*
 * fila.c — IMPLEMENTAÇÃO do TAD Fila de Pedidos com VETOR CIRCULAR
 * (alocação sequencial — Aula 07; vetor alocado dinamicamente — Aula 10).
 *
 * Desenho de uma fila de capacidade 5 com 3 pedidos (A, B, C):
 *
 *   índice:    0     1     2     3     4
 *   dados:  [  -  ][  A  ][  B  ][  C  ][  -  ]
 *                    ^                   ^
 *                 primeiro             ultimo     quantidade = 3
 *
 *   - primeiro: índice de quem está na FRENTE (o próximo a sair);
 *   - ultimo:   índice da PRÓXIMA POSIÇÃO LIVRE (onde entra o próximo);
 *   - quantidade: quantos elementos há (é ela que diz se está vazia/cheia).
 *
 * Por que CIRCULAR? Se os índices só andassem para a direita, depois de
 * algumas remoções o 'ultimo' chegaria ao fim do vetor mesmo com espaço
 * livre no começo. Com o operador % (resto da divisão), o índice que passa
 * do fim "dá a volta" para o 0:  com capacidade 5 -> 3, 4, 0, 1, 2, ...
 *
 * ATENÇÃO (pegadinha clássica): NÃO dá para usar "primeiro == ultimo" para
 * saber se está vazia, porque isso acontece tanto vazia quanto cheia.
 * Por isso guardamos 'quantidade'.
 */

#include <stdlib.h>   /* malloc, free */
#include "fila.h"

/* Definição REAL da struct (escondida de quem só inclui fila.h). */
struct fila {
    Pedido *dados;    /* vetor de pedidos, alocado com malloc no fila_criar */
    int capacidade;   /* tamanho do vetor 'dados'                            */
    int primeiro;     /* índice da frente                                    */
    int ultimo;       /* índice da próxima posição livre (fim da fila)       */
    int quantidade;   /* quantos pedidos há na fila                          */
};

Fila *fila_criar(int capacidade) {
    if (capacidade <= 0) {
        return NULL;                       /* capacidade sem sentido */
    }

    /* 1) Aloca a "caixa" da fila (a struct com os índices). */
    Fila *f = malloc(sizeof(Fila));
    if (f == NULL) {
        return NULL;                       /* faltou memória */
    }

    /* 2) Aloca o vetor onde os pedidos ficam guardados. */
    f->dados = malloc(capacidade * sizeof(Pedido));
    if (f->dados == NULL) {
        free(f);                           /* desfaz o passo 1 antes de sair */
        return NULL;
    }

    /* 3) Estado inicial: fila vazia. */
    f->capacidade = capacidade;
    f->primeiro = 0;
    f->ultimo = 0;
    f->quantidade = 0;
    return f;
}

void fila_destruir(Fila *f) {
    if (f == NULL) {
        return;
    }
    /* Libera na ordem inversa da criação: primeiro o vetor, depois a struct.
     * (Se liberássemos a struct antes, perderíamos o ponteiro f->dados.) */
    free(f->dados);
    free(f);
}

bool fila_vazia(const Fila *f) {
    return f->quantidade == 0;
}

bool fila_cheia(const Fila *f) {
    return f->quantidade == f->capacidade;
}

int fila_tamanho(const Fila *f) {
    return f->quantidade;
}

bool fila_enfileirar(Fila *f, Pedido p) {
    if (fila_cheia(f)) {
        return false;                                /* não cabe: nada muda */
    }
    f->dados[f->ultimo] = p;                         /* 1º grava no fim     */
    f->ultimo = (f->ultimo + 1) % f->capacidade;     /* 2º anda (com volta) */
    f->quantidade++;                                 /* 3º conta            */
    return true;
}

bool fila_desenfileirar(Fila *f, Pedido *removido) {
    if (fila_vazia(f)) {
        return false;                                /* nada para remover */
    }
    if (removido != NULL) {
        *removido = f->dados[f->primeiro];           /* entrega quem saiu */
    }
    /* Não precisa "apagar" o valor do vetor: basta andar o índice. A posição
     * antiga vira espaço livre e será sobrescrita no futuro. */
    f->primeiro = (f->primeiro + 1) % f->capacidade;
    f->quantidade--;
    return true;
}

bool fila_frente(const Fila *f, Pedido *frente) {
    if (fila_vazia(f) || frente == NULL) {
        return false;
    }
    *frente = f->dados[f->primeiro];
    return true;
}

bool fila_consultar(const Fila *f, int posicao, Pedido *saida) {
    if (saida == NULL || posicao < 0 || posicao >= f->quantidade) {
        return false;                                /* posição não existe */
    }
    /* A posição 'posicao' (contada a partir da frente) fica 'posicao' casas
     * depois de 'primeiro' — e o % faz a volta no fim do vetor. */
    *saida = f->dados[(f->primeiro + posicao) % f->capacidade];
    return true;
}
