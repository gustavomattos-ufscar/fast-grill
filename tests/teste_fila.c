/*
 * teste_fila.c — Testes do TAD Fila de Pedidos.
 *
 * Usa uma fila de capacidade 3 (menor que a do jogo) para chegar rápido
 * nos casos de limite: fila cheia e "volta" do índice circular.
 * Compilar e rodar: make test
 */

#include <stdio.h>
#include "teste.h"
#include "../src/fila.h"

/* Cria um pedido simples só para teste: o que importa é o número.
 * [v2] pedido_criar agora recebe as regras (aqui, as do Fácil). */
static Pedido pedido_teste(int numero) {
    Receita r;
    RegrasPedido regras = { RECEITA_MAX, NUM_INGREDIENTES_BASICOS, 8, 3, 10 };
    cardapio_receita(0, &r);
    return pedido_criar(numero, r, &regras);
}

int main(void) {
    Pedido p;
    int i;

    /* ------------------------------------------------------------------ */
    secao("Criacao");
    verifica(fila_criar(0) == NULL, "capacidade 0 e recusada (retorna NULL)");
    verifica(fila_criar(-2) == NULL, "capacidade negativa e recusada");

    Fila *f = fila_criar(3);
    verifica(f != NULL, "fila_criar(3) devolve uma fila");
    verifica(fila_vazia(f), "fila recem-criada esta vazia");
    verifica(!fila_cheia(f), "fila recem-criada nao esta cheia");
    verifica(fila_tamanho(f) == 0, "tamanho inicial e 0");

    /* ------------------------------------------------------------------ */
    secao("Operacoes em fila vazia (underflow)");
    verifica(!fila_frente(f, &p), "frente falha com a fila vazia");
    verifica(!fila_desenfileirar(f, &p), "desenfileirar falha com a fila vazia");
    verifica(!fila_consultar(f, 0, &p), "consultar posicao 0 falha com a fila vazia");
    verifica(fila_tamanho(f) == 0, "tamanho continua 0 apos as falhas");

    /* ------------------------------------------------------------------ */
    secao("Ordem FIFO e fila cheia (overflow)");
    verifica(fila_enfileirar(f, pedido_teste(1)), "enfileira #1");
    verifica(fila_frente(f, &p) && p.numero == 1, "frente e #1");
    verifica(fila_enfileirar(f, pedido_teste(2)), "enfileira #2");
    verifica(fila_enfileirar(f, pedido_teste(3)), "enfileira #3");
    verifica(fila_cheia(f), "com 3 pedidos a fila esta cheia");
    verifica(!fila_enfileirar(f, pedido_teste(4)), "enfileirar #4 falha (cheia)");
    verifica(fila_tamanho(f) == 3, "tamanho continua 3");
    verifica(fila_frente(f, &p) && p.numero == 1, "frente continua #1 (nao foi sobrescrita)");

    verifica(fila_desenfileirar(f, &p) && p.numero == 1, "sai #1 (o primeiro que entrou)");
    verifica(fila_desenfileirar(f, &p) && p.numero == 2, "sai #2");
    verifica(fila_tamanho(f) == 1, "sobrou 1 pedido");

    /* ------------------------------------------------------------------ */
    secao("Volta do indice circular");
    /* Agora primeiro = 2 e ultimo = 0 (ja deu a volta). Enfileirar mais
     * dois obriga os indices a passar do fim do vetor para o comeco. */
    verifica(fila_enfileirar(f, pedido_teste(4)), "enfileira #4 (vai para o indice 0)");
    verifica(fila_enfileirar(f, pedido_teste(5)), "enfileira #5 (vai para o indice 1)");
    verifica(fila_cheia(f), "fila cheia de novo");
    verifica(fila_consultar(f, 0, &p) && p.numero == 3, "consultar(0) = #3 (frente)");
    verifica(fila_consultar(f, 1, &p) && p.numero == 4, "consultar(1) = #4");
    verifica(fila_consultar(f, 2, &p) && p.numero == 5, "consultar(2) = #5 (fim)");
    verifica(!fila_consultar(f, 3, &p), "consultar(3) falha (posicao nao existe)");
    verifica(!fila_consultar(f, -1, &p), "consultar(-1) falha");
    verifica(fila_tamanho(f) == 3, "consultar nao altera o tamanho");
    verifica(fila_desenfileirar(f, &p) && p.numero == 3, "sai #3");
    verifica(fila_desenfileirar(f, &p) && p.numero == 4, "sai #4");
    verifica(fila_desenfileirar(f, NULL), "desenfileirar aceita NULL (descarta #5)");
    verifica(fila_vazia(f), "fila vazia no final");

    /* ------------------------------------------------------------------ */
    secao("Muitas voltas (100 entradas e saidas)");
    bool ordem_ok = true;
    for (i = 1; i <= 100; i++) {
        fila_enfileirar(f, pedido_teste(i));
        if (i % 2 == 0) {            /* a cada 2 que entram, 2 saem */
            Pedido a, b;
            fila_desenfileirar(f, &a);
            fila_desenfileirar(f, &b);
            if (a.numero != i - 1 || b.numero != i) {
                ordem_ok = false;
            }
        }
    }
    verifica(ordem_ok, "ordem FIFO mantida em 50 voltas do vetor");
    verifica(fila_vazia(f), "fila termina vazia");

    fila_destruir(f);
    f = NULL;
    fila_destruir(NULL);   /* nao pode quebrar */
    verifica(true, "fila_destruir libera a fila e aceita NULL");

    return resumo_testes("teste_fila");
}
