/*
 * teste.h — Mini "biblioteca" de testes feita pelo grupo.
 *
 * Ideia: cada teste é uma condição que DEVERIA ser verdadeira.
 *   verifica(fila_vazia(f), "fila recem-criada esta vazia");
 * Se for verdadeira, imprime [ OK ]; se for falsa, imprime [FALHOU].
 * No fim, resumo_testes() mostra quantos passaram e devolve o código de
 * saída do programa (0 = tudo certo, 1 = algo falhou). O Makefile usa
 * esse código para saber se o "make test" passou.
 */

#ifndef TESTE_H
#define TESTE_H

#include <stdbool.h>

/* Registra um teste: imprime o resultado e conta acertos/falhas. */
void verifica(bool condicao, const char *descricao);

/* Imprime um título para separar grupos de testes na saída. */
void secao(const char *titulo);

/* Imprime o total e devolve 0 se todos passaram, 1 se algum falhou. */
int resumo_testes(const char *nome_do_arquivo);

#endif /* TESTE_H */
