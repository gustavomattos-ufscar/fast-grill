/*
 * fila.h — INTERFACE do TAD Fila de Pedidos.
 *
 * O QUE É: uma fila FIFO (First In, First Out — o primeiro a entrar é o
 * primeiro a sair). No jogo, os clientes entram no FIM da fila quando
 * chegam e só o cliente da FRENTE pode ser atendido ou desistir.
 *
 * TIPO OPACO (Aula 06): aqui no .h só dizemos que "existe uma struct fila".
 * Os campos (vetor, índices, quantidade) ficam escondidos no fila.c.
 * Assim, quem usa a fila (jogo.c, ui.c) NÃO consegue mexer nos campos e
 * só pode usar as funções abaixo. Isso é o encapsulamento do TAD.
 * Vantagem prática: na Fase 3 podemos trocar o vetor circular por uma
 * lista encadeada mexendo só no fila.c, sem alterar o resto do jogo.
 *
 * Como o tipo é opaco, a fila precisa ser criada com malloc (fila_criar)
 * e liberada com free (fila_destruir) — Aula 09.
 *
 * Estilo das funções (Aula 10): as operações que podem falhar devolvem
 * bool (true = deu certo, false = não deu) e entregam o resultado por um
 * parâmetro de saída (ponteiro).
 */

#ifndef FILA_H
#define FILA_H

#include <stdbool.h>
#include "pedido.h"   /* tipo do elemento: Pedido */

/* Declaração do tipo opaco: "Fila" é um apelido para "struct fila",
 * cujos campos só o fila.c conhece. */
typedef struct fila Fila;

/* Cria uma fila vazia que cabe 'capacidade' pedidos.
 * Retorna o ponteiro para a fila, ou NULL se capacidade <= 0 ou se faltar
 * memória. Complexidade: O(1). */
Fila *fila_criar(int capacidade);

/* Libera toda a memória da fila. Depois de chamar, NÃO use mais o
 * ponteiro (boa prática: atribuir NULL a ele). Aceita NULL. O(1). */
void fila_destruir(Fila *f);

/* Insere o pedido 'p' no FIM da fila.
 * Retorna false se a fila estiver cheia (nada muda). O(1). */
bool fila_enfileirar(Fila *f, Pedido p);

/* Remove o pedido da FRENTE da fila.
 * Se 'removido' não for NULL, copia o pedido removido para *removido.
 * Retorna false se a fila estiver vazia (nada muda). O(1). */
bool fila_desenfileirar(Fila *f, Pedido *removido);

/* Copia para *frente o pedido da FRENTE, SEM removê-lo.
 * Retorna false se a fila estiver vazia. O(1). */
bool fila_frente(const Fila *f, Pedido *frente);

/* true se não há nenhum pedido. O(1). */
bool fila_vazia(const Fila *f);

/* true se não cabe mais nenhum pedido. O(1). */
bool fila_cheia(const Fila *f);

/* Quantos pedidos estão na fila agora. O(1). */
int fila_tamanho(const Fila *f);

/* CONSULTA SOMENTE LEITURA, usada APENAS pela interface para desenhar a
 * fila inteira na tela. 'posicao' 0 é a frente, 1 é o segundo, ...,
 * fila_tamanho(f) - 1 é o último.
 * Copia o pedido para *saida e retorna true; retorna false se a posição
 * não existir. Não altera a fila (por isso o 'const'). O(1).
 * As REGRAS do jogo (jogo.c) nunca usam esta função: elas só usam frente,
 * enfileirar e desenfileirar, respeitando o comportamento FIFO. */
bool fila_consultar(const Fila *f, int posicao, Pedido *saida);

#endif /* FILA_H */
