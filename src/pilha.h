/*
 * pilha.h — INTERFACE do TAD Pilha de Prato.
 *
 * O QUE É: uma pilha LIFO (Last In, First Out — o último a entrar é o
 * primeiro a sair). No jogo, o prato é montado de baixo para cima e a
 * ação "desfazer" remove apenas o ÚLTIMO ingrediente colocado: é
 * exatamente o comportamento de uma pilha.
 *
 * Assim como a Fila, é um TIPO OPACO: os campos ficam no pilha.c e quem
 * usa só enxerga as funções abaixo. A pilha é criada com malloc
 * (pilha_criar) e liberada com free (pilha_destruir).
 */

#ifndef PILHA_H
#define PILHA_H

#include <stdbool.h>
#include "pedido.h"   /* tipo do elemento: Ingrediente */

typedef struct pilha Pilha;   /* tipo opaco */

/* Cria uma pilha vazia que cabe 'capacidade' ingredientes.
 * Retorna NULL se capacidade <= 0 ou se faltar memória. O(1). */
Pilha *pilha_criar(int capacidade);

/* Libera toda a memória da pilha. Aceita NULL. O(1). */
void pilha_destruir(Pilha *p);

/* Coloca 'ing' no TOPO da pilha.
 * Retorna false se a pilha estiver cheia (nada muda). O(1). */
bool pilha_empilhar(Pilha *p, Ingrediente ing);

/* Remove o ingrediente do TOPO.
 * Se 'removido' não for NULL, copia o ingrediente removido para *removido.
 * Retorna false se a pilha estiver vazia (nada muda). O(1). */
bool pilha_desempilhar(Pilha *p, Ingrediente *removido);

/* Copia para *topo o ingrediente do TOPO, SEM removê-lo.
 * Retorna false se a pilha estiver vazia. O(1). */
bool pilha_topo(const Pilha *p, Ingrediente *topo);

/* true se não há nenhum ingrediente. O(1). */
bool pilha_vazia(const Pilha *p);

/* true se não cabe mais nenhum ingrediente. O(1). */
bool pilha_cheia(const Pilha *p);

/* Quantos ingredientes há na pilha agora. O(1). */
int pilha_tamanho(const Pilha *p);

/* CONSULTA SOMENTE LEITURA, usada APENAS pela interface para desenhar o
 * prato inteiro. 'posicao' 0 é a BASE (primeiro ingrediente colocado) e
 * pilha_tamanho(p) - 1 é o TOPO.
 * Retorna false se a posição não existir. Não altera a pilha. O(1).
 * As regras do jogo (jogo.c) nunca usam esta função: só empilhar,
 * desempilhar e topo (comportamento LIFO). */
bool pilha_consultar(const Pilha *p, int posicao, Ingrediente *saida);

#endif /* PILHA_H */
