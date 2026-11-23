/*
 * pilha.c — IMPLEMENTAÇÃO do TAD Pilha de Prato com VETOR
 * (alocação sequencial — Aula 07; vetor alocado dinamicamente — Aula 10).
 *
 * Desenho de uma pilha de capacidade 7 com 3 ingredientes:
 *
 *   índice:   6 [  -     ]
 *             5 [  -     ]
 *             4 [  -     ]
 *             3 [  -     ]
 *             2 [ Queijo ]  <- topo = 2   (último colocado)
 *             1 [ Carne  ]
 *             0 [ Pao    ]  <- base       (primeiro colocado)
 *
 * Convenção dos slides: 'topo' é o ÍNDICE do elemento do topo.
 *   - vazia:  topo == -1
 *   - cheia:  topo == capacidade - 1
 *   - tamanho = topo + 1
 */

#include <stdlib.h>   /* malloc, free */
#include "pilha.h"

/* Definição REAL da struct (escondida de quem só inclui pilha.h). */
struct pilha {
    Ingrediente *dados;  /* vetor alocado com malloc no pilha_criar */
    int capacidade;      /* tamanho do vetor 'dados'                 */
    int topo;            /* índice do topo; -1 quando vazia          */
};

Pilha *pilha_criar(int capacidade) {
    if (capacidade <= 0) {
        return NULL;
    }

    Pilha *p = malloc(sizeof(Pilha));          /* 1) a struct */
    if (p == NULL) {
        return NULL;
    }

    p->dados = malloc(capacidade * sizeof(Ingrediente));   /* 2) o vetor */
    if (p->dados == NULL) {
        free(p);                               /* desfaz o passo 1 */
        return NULL;
    }

    p->capacidade = capacidade;
    p->topo = -1;                              /* 3) começa vazia */
    return p;
}

void pilha_destruir(Pilha *p) {
    if (p == NULL) {
        return;
    }
    free(p->dados);   /* primeiro o vetor... */
    free(p);          /* ...depois a struct  */
}

bool pilha_vazia(const Pilha *p) {
    return p->topo == -1;
}

bool pilha_cheia(const Pilha *p) {
    return p->topo == p->capacidade - 1;
}

int pilha_tamanho(const Pilha *p) {
    return p->topo + 1;   /* topo é índice: com topo = 2 há 3 elementos */
}

bool pilha_empilhar(Pilha *p, Ingrediente ing) {
    if (pilha_cheia(p)) {
        return false;            /* não cabe: nada muda */
    }
    p->topo++;                   /* 1º sobe o topo...            */
    p->dados[p->topo] = ing;     /* 2º ...e grava na nova posição */
    return true;
}

bool pilha_desempilhar(Pilha *p, Ingrediente *removido) {
    if (pilha_vazia(p)) {
        return false;            /* nada para remover */
    }
    if (removido != NULL) {
        *removido = p->dados[p->topo];   /* entrega quem saiu */
    }
    p->topo--;                   /* não precisa apagar: basta descer o topo */
    return true;
}

bool pilha_topo(const Pilha *p, Ingrediente *topo) {
    if (pilha_vazia(p) || topo == NULL) {
        return false;
    }
    *topo = p->dados[p->topo];
    return true;
}

bool pilha_consultar(const Pilha *p, int posicao, Ingrediente *saida) {
    if (saida == NULL || posicao < 0 || posicao > p->topo) {
        return false;            /* posição não existe */
    }
    *saida = p->dados[posicao];  /* posição 0 = base */
    return true;
}
