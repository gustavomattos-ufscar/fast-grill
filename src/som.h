/*
 * som.h — [v2] MÓDULO NOVO: efeitos sonoros do jogo.
 *
 * Os sons NÃO vêm de arquivos .wav/.mp3: eles são GERADOS pelo código
 * (síntese simples de ondas, explicada em som.c). Vantagens:
 *   - nenhum arquivo extra para carregar ou perder;
 *   - nenhum problema de direitos autorais de sons de terceiros;
 *   - dá para ajustar cada som mudando só alguns números.
 *
 * Quem decide QUANDO tocar é o main.c, olhando os Eventos que o núcleo
 * (jogo.c) anota. Este módulo só sabe TOCAR. Ele usa a raylib, então,
 * assim como ui.c, não entra nos testes nem no modo texto.
 */

#ifndef SOM_H
#define SOM_H

#include <stdbool.h>

/* Os efeitos disponíveis. */
typedef enum {
    SOM_EMPILHAR,   /* "pop" curto: ingrediente no prato                 */
    SOM_DESFAZER,   /* "pop" descendo: ingrediente retirado              */
    SOM_SERVIR,     /* arpejo subindo: pedido entregue                   */
    SOM_ERRO,       /* zumbido grave: ação recusada / prato errado       */
    SOM_CHEGADA,    /* campainha "plim-plom": cliente entrou na fila     */
    SOM_VIDA,       /* três notas descendo: perdeu uma vida              */
    SOM_NIVEL,      /* arpejo rápido e agudo: subiu de nível             */
    SOM_FIM,        /* notas longas descendo: fim de jogo                */
    SOM_MENU,       /* clique: troca de opção no menu                    */
    SOM_INICIAR,    /* duas notas: começou a partida                     */
    NUM_SONS        /* truque: vale a quantidade de sons acima (10)      */
} TipoSom;

/* Liga o dispositivo de áudio e gera todos os sons.
 * Chamar UMA vez, depois de InitWindow. Se o computador não tiver
 * saída de áudio, o jogo continua funcionando, só que mudo. */
void som_iniciar(void);

/* Libera os sons e desliga o áudio. Chamar antes de CloseWindow. */
void som_encerrar(void);

/* Toca um efeito (se o som estiver ligado). */
void som_tocar(TipoSom tipo);

/* Liga/desliga todos os sons (tecla M). */
void som_alternar(void);

/* true se o som está ligado (a tela mostra esse estado). */
bool som_esta_ligado(void);

#endif /* SOM_H */
