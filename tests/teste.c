/*
 * teste.c — Implementação da mini biblioteca de testes (ver teste.h).
 */

#include <stdio.h>
#include "teste.h"

/* Contadores globais, visíveis só neste arquivo. */
static int total = 0;
static int falhas = 0;

void verifica(bool condicao, const char *descricao) {
    total++;
    if (condicao) {
        printf("  [ OK ]  %s\n", descricao);
    } else {
        falhas++;
        printf("  [FALHOU] %s\n", descricao);
    }
}

void secao(const char *titulo) {
    printf("\n== %s ==\n", titulo);
}

int resumo_testes(const char *nome_do_arquivo) {
    printf("\n%s: %d de %d testes passaram", nome_do_arquivo, total - falhas, total);
    if (falhas == 0) {
        printf(" -> TUDO CERTO\n\n");
        return 0;
    }
    printf(" -> %d FALHA(S)\n\n", falhas);
    return 1;
}
