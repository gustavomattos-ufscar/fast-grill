/*
 * jogo.h — NÚCLEO DA PARTIDA: estado do jogo + regras.
 *
 * Este módulo é o "cérebro" do Fast Grill. Ele junta os dois TADs:
 *   - Fila de Pedidos (fila.h): clientes esperando, na ordem de chegada;
 *   - Pilha de Prato  (pilha.h): o lanche sendo montado.
 *
 * Regra de ouro da arquitetura: SÓ ESTE MÓDULO ALTERA A PARTIDA.
 *   - A interface (ui.c) recebe 'const Partida *' -> o compilador impede
 *     que ela mude qualquer campo; ela só LÊ para desenhar.
 *   - O main.c lê o que o jogador fez e chama jogo_empilhar, jogo_servir...
 *
 * O núcleo não conhece a raylib nem o relógio: ele só "conta ticks".
 * Quem decide quando um tick acontece (a cada 1 segundo) é o main.c.
 * Por isso toda a regra pode ser testada no terminal, sem janela.
 *
 * [v2] MUDANÇAS DA VERSÃO 2:
 *   1) DIFICULDADE: enum Dificuldade + struct ConfigDificuldade. Os
 *      números que eram #define fixos (intervalo de chegada, tamanho da
 *      fila, pedidos por nível...) agora vêm da configuração escolhida.
 *      O Fácil usa EXATAMENTE os valores da versão 1.
 *   2) EVENTOS: struct Eventos dentro da Partida. O núcleo marca o que
 *      aconteceu (cliente chegou, perdeu vida, serviu...) e o main.c usa
 *      isso para tocar os efeitos sonoros. O núcleo continua sem saber o
 *      que é som ou raylib: ele só "anota" os acontecimentos.
 */

#ifndef JOGO_H
#define JOGO_H

#include <stdbool.h>
#include "pedido.h"
#include "fila.h"
#include "pilha.h"

/* ------------------------------------------------------------------------
 * Regras numéricas FIXAS (iguais em todas as dificuldades)
 * ------------------------------------------------------------------------ */
#define VIDAS_INICIAIS 3
#define PEDIDOS_INICIAIS 2          /* pedidos já na fila no começo         */
#define MAX_PRATO RECEITA_MAX       /* prato aguenta 7 ingredientes         */
#define TAM_MENSAGEM 96             /* tamanho do texto de aviso            */

/* [v2] Saíram daqui (agora estão na ConfigDificuldade, em jogo.c):
 *   MAX_PEDIDOS_FILA, INTERVALO_INICIAL, INTERVALO_MINIMO,
 *   PEDIDOS_POR_NIVEL, TAMANHO_RECEITA_INICIAL. */

/* ------------------------------------------------------------------------
 * [v2] Dificuldade
 * ------------------------------------------------------------------------ */
typedef enum {
    DIFICULDADE_FACIL,     /* 0 */
    DIFICULDADE_MEDIO,     /* 1 */
    DIFICULDADE_DIFICIL    /* 2 */
} Dificuldade;

#define NUM_DIFICULDADES 3

/* [v2] Tudo o que muda de uma dificuldade para outra.
 * Os valores de cada uma estão na tabela CONFIGS[] em jogo.c. */
typedef struct {
    const char *nome;               /* "Facil", "Medio", "Dificil"          */
    int ingredientes_disponiveis;   /* 5, 7 ou 8 (botões na tela)           */
    int paciencia_base;             /* paciência = base + por_ingrediente   */
    int paciencia_por_ingrediente;  /*             x tamanho da receita     */
    int pontos_por_ingrediente;     /* pontos ganhos por ingrediente        */
    int intervalo_inicial;          /* ticks entre chegadas no nível 1      */
    int intervalo_minimo;           /* o intervalo nunca fica menor         */
    int tamanho_receita_inicial;    /* maior receita no nível 1             */
    int pedidos_por_nivel;          /* servidos para subir de nível         */
    int max_pedidos_fila;           /* fila lotada com quantos pedidos      */
    int penalidade_erro;            /* paciência perdida ao servir errado   */
} ConfigDificuldade;

/* ------------------------------------------------------------------------
 * [v2] Eventos: "anotações" do que aconteceu desde a última limpeza.
 * O núcleo liga (true) cada campo quando o fato acontece; o main.c lê,
 * toca o som correspondente e chama jogo_limpar_eventos.
 * ------------------------------------------------------------------------ */
typedef struct {
    bool empilhou;        /* um ingrediente foi para o prato           */
    bool desfez;          /* o topo do prato foi retirado              */
    bool serviu;          /* um pedido foi entregue corretamente       */
    bool errou;           /* uma ação foi recusada (prato errado etc.) */
    bool cliente_chegou;  /* um cliente entrou na fila                 */
    bool perdeu_vida;     /* desistência ou fila lotada                */
    bool subiu_nivel;     /* o nível aumentou                          */
    bool terminou;        /* as vidas acabaram                         */
} Eventos;

/* ------------------------------------------------------------------------
 * Partida: TODO o estado do jogo em um lugar só.
 *
 * Diferente da Fila e da Pilha, esta struct NÃO é opaca: a interface
 * precisa ler pontos, vidas, mensagem... para desenhar. A proteção vem do
 * 'const': ui.c recebe 'const Partida *' e não consegue alterar nada.
 * ------------------------------------------------------------------------ */
typedef struct {
    Fila  *fila;              /* TAD Fila: pedidos esperando (FIFO)          */
    Pilha *prato;             /* TAD Pilha: lanche em montagem (LIFO)        */

    Dificuldade dificuldade;  /* [v2] dificuldade escolhida no menu          */
    ConfigDificuldade config; /* [v2] cópia dos números dessa dificuldade    */

    int acertos;              /* quantos ingredientes da BASE do prato batem
                                 com a receita do pedido da frente (ver
                                 explicação em jogo.c)                       */
    int paciencia_frente;     /* ticks que faltam para o cliente da frente
                                 desistir (só o da frente perde paciência)   */

    int tick;                 /* quantos ticks já se passaram                */
    int intervalo_chegada;    /* ticks entre chegadas (diminui com o nível)  */
    int ticks_para_chegada;   /* contagem regressiva até o próximo cliente   */
    int nivel;                /* 0, 1, 2... sobe a cada config.pedidos_por_nivel */

    int pontos;
    int vidas;
    int servidos;             /* pedidos entregues corretamente              */
    int perdidos;             /* clientes que foram embora                   */

    int proximo_numero;       /* número do próximo cliente a chegar          */
    unsigned int semente;     /* estado do sorteio de pedidos                */
    bool terminou;            /* true quando as vidas acabam                 */
    char mensagem[TAM_MENSAGEM]; /* último aviso para o jogador              */
    Eventos eventos;          /* [v2] o que aconteceu (para os sons)         */
} Partida;

/* ------------------------------------------------------------------------
 * Operações do núcleo
 * ------------------------------------------------------------------------ */

/* [v2] Devolve a configuração (os números) de uma dificuldade.
 * Usada pelo jogo_iniciar e pelo menu, para mostrar as diferenças. */
ConfigDificuldade jogo_config(Dificuldade d);

/* Prepara uma partida nova em *p: 3 vidas, 0 pontos, prato vazio e 2
 * pedidos na fila. 'semente' define a sequência de pedidos (mesma semente
 * = mesma partida). Retorna false se faltar memória para a fila/pilha.
 * [v2] ganhou o parâmetro 'dificuldade'. */
bool jogo_iniciar(Partida *p, unsigned int semente, Dificuldade dificuldade);

/* Libera a memória da fila e da pilha. Chamar ao fim de cada partida. */
void jogo_destruir(Partida *p);

/* Avança o tempo em 1 tick (o main.c chama 1 vez por segundo):
 *   1) o cliente da frente perde 1 de paciência; se zerar, ele desiste:
 *      sai da fila, o prato é descartado e o jogador perde 1 vida;
 *   2) a contagem para o próximo cliente diminui; se zerar, chega um
 *      cliente novo (ou, se a fila estiver lotada, ele vai embora e o
 *      jogador perde 1 vida).
 * Não faz nada se a partida já terminou. */
void jogo_tick(Partida *p);

/* Ação do jogador: coloca 'ing' no topo do prato.
 * Falha (retorna false e explica em p->mensagem) se não houver pedido,
 * se o prato estiver cheio, se o ingrediente não existir nesta
 * dificuldade [v2] ou se a partida tiver terminado. */
bool jogo_empilhar(Partida *p, Ingrediente ing);

/* Ação do jogador: tira o ingrediente do topo do prato (desfazer).
 * Falha se o prato estiver vazio ou se a partida tiver terminado. */
bool jogo_desfazer(Partida *p);

/* Ação do jogador: entrega o prato ao cliente da frente.
 * Só funciona se o prato for IDÊNTICO à receita, na mesma ordem. Nesse
 * caso: soma os pontos, o pedido sai da fila e o prato é esvaziado.
 * Se estiver errado, nada muda no prato e p->mensagem diz o problema.
 * [v2] No Médio e no Difícil, servir errado custa paciência do cliente
 * da frente (config.penalidade_erro). */
bool jogo_servir(Partida *p);

/* true quando a partida acabou (vidas chegaram a 0). */
bool jogo_terminou(const Partida *p);

/* Tamanho máximo de receita que pode aparecer no nível atual. */
int jogo_tamanho_receita_max(const Partida *p);

/* [v2] Zera todos os eventos (chamado pelo main.c depois de tocar os sons). */
void jogo_limpar_eventos(Partida *p);

#endif /* JOGO_H */
