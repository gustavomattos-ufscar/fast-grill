/*
 * teste_pilha.c — Testes do TAD Pilha de Prato.
 * Compilar e rodar: make test
 */

#include <stdio.h>
#include "teste.h"
#include "../src/pilha.h"

int main(void) {
    Ingrediente ing;

    /* ------------------------------------------------------------------ */
    secao("Criacao");
    verifica(pilha_criar(0) == NULL, "capacidade 0 e recusada (retorna NULL)");

    Pilha *p = pilha_criar(3);
    verifica(p != NULL, "pilha_criar(3) devolve uma pilha");
    verifica(pilha_vazia(p), "pilha recem-criada esta vazia");
    verifica(!pilha_cheia(p), "pilha recem-criada nao esta cheia");
    verifica(pilha_tamanho(p) == 0, "tamanho inicial e 0");

    /* ------------------------------------------------------------------ */
    secao("Operacoes em pilha vazia (underflow)");
    verifica(!pilha_topo(p, &ing), "topo falha com a pilha vazia");
    verifica(!pilha_desempilhar(p, &ing), "desempilhar falha com a pilha vazia");
    verifica(!pilha_consultar(p, 0, &ing), "consultar(0) falha com a pilha vazia");

    /* ------------------------------------------------------------------ */
    secao("Ordem LIFO e pilha cheia (overflow)");
    verifica(pilha_empilhar(p, PAO), "empilha Pao");
    verifica(pilha_topo(p, &ing) && ing == PAO, "topo e Pao");
    verifica(pilha_empilhar(p, CARNE), "empilha Carne");
    verifica(pilha_topo(p, &ing) && ing == CARNE, "topo e Carne (o ultimo que entrou)");
    verifica(pilha_empilhar(p, QUEIJO), "empilha Queijo");
    verifica(pilha_cheia(p), "com 3 itens a pilha esta cheia");
    verifica(!pilha_empilhar(p, TOMATE), "empilhar Tomate falha (cheia)");
    verifica(pilha_topo(p, &ing) && ing == QUEIJO, "topo continua Queijo");
    verifica(pilha_tamanho(p) == 3, "tamanho e 3");

    /* ------------------------------------------------------------------ */
    secao("Consulta somente leitura (base -> topo)");
    verifica(pilha_consultar(p, 0, &ing) && ing == PAO, "consultar(0) = Pao (base)");
    verifica(pilha_consultar(p, 1, &ing) && ing == CARNE, "consultar(1) = Carne");
    verifica(pilha_consultar(p, 2, &ing) && ing == QUEIJO, "consultar(2) = Queijo (topo)");
    verifica(!pilha_consultar(p, 3, &ing), "consultar(3) falha (posicao nao existe)");
    verifica(pilha_tamanho(p) == 3, "consultar nao altera o tamanho");

    /* ------------------------------------------------------------------ */
    secao("Desempilhar na ordem inversa");
    verifica(pilha_desempilhar(p, &ing) && ing == QUEIJO, "sai Queijo (ultimo que entrou)");
    verifica(pilha_desempilhar(p, &ing) && ing == CARNE, "sai Carne");
    verifica(pilha_desempilhar(p, NULL), "desempilhar aceita NULL (descarta Pao)");
    verifica(pilha_vazia(p), "pilha vazia no final");
    verifica(!pilha_desempilhar(p, &ing), "desempilhar de novo falha");

    /* ------------------------------------------------------------------ */
    secao("Reuso depois de esvaziar");
    verifica(pilha_empilhar(p, ALFACE), "empilha Alface na pilha reaproveitada");
    verifica(pilha_tamanho(p) == 1, "tamanho volta a ser 1");
    verifica(pilha_topo(p, &ing) && ing == ALFACE, "topo e Alface");

    pilha_destruir(p);
    p = NULL;
    pilha_destruir(NULL);
    verifica(true, "pilha_destruir libera a pilha e aceita NULL");

    return resumo_testes("teste_pilha");
}
