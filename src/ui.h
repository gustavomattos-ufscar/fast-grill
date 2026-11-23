/*
 * ui.h — INTERFACE GRÁFICA (desenho das telas e leitura do teclado/mouse).
 *
 * Divisão de responsabilidades:
 *   - ui_ler_acao: olha o teclado e o mouse e devolve O QUE o jogador quer
 *     fazer (uma Acao). Ela NÃO executa a ação.
 *   - main.c recebe a Acao e chama o núcleo (jogo_empilhar, jogo_servir...).
 *   - ui_desenhar: desenha a tela a partir do estado da partida, que ela
 *     recebe como 'const Partida *' (somente leitura).
 *
 * Assim a interface nunca altera o jogo diretamente: só o núcleo altera.
 * Este é o ÚNICO módulo (junto com main.c e som.c) que usa a raylib.
 *
 * [v2] MUDANÇAS DA VERSÃO 2:
 *   - menu com SELETOR DE DIFICULDADE (Fácil / Médio / Difícil);
 *   - novas ações: escolher dificuldade, voltar ao menu, ligar/desligar som;
 *   - ui_ler_acao e ui_desenhar recebem mais informações (a partida, a
 *     dificuldade selecionada no menu e se o som está ligado);
 *   - a quantidade de botões de ingrediente depende da dificuldade (5, 7 ou 8).
 */

#ifndef UI_H
#define UI_H

#include "jogo.h"

#define LARGURA_TELA 1000
#define ALTURA_TELA 640

/* Em qual tela o jogo está. */
typedef enum {
    TELA_MENU,     /* tela inicial: instruções + escolha da dificuldade */
    TELA_JOGO,     /* partida em andamento                               */
    TELA_FIM       /* placar final                                       */
} Tela;

/* O que o jogador pediu neste quadro (frame). */
typedef enum {
    ACAO_NENHUMA,
    ACAO_EMPILHAR,              /* qual ingrediente: campo 'ingrediente'    */
    ACAO_DESFAZER,
    ACAO_SERVIR,
    ACAO_CONFIRMAR,             /* ENTER/clique: começar ou jogar de novo   */
    ACAO_ESCOLHER_DIFICULDADE,  /* [v2] qual: campo 'dificuldade'           */
    ACAO_VOLTAR_MENU,           /* [v2] da tela de fim para o menu          */
    ACAO_ALTERNAR_SOM           /* [v2] tecla M em qualquer tela            */
} TipoAcao;

typedef struct {
    TipoAcao tipo;
    Ingrediente ingrediente;    /* só vale quando tipo == ACAO_EMPILHAR     */
    Dificuldade dificuldade;    /* [v2] só vale em ACAO_ESCOLHER_DIFICULDADE */
} Acao;

/* Lê teclado e mouse e devolve a ação do jogador (ou ACAO_NENHUMA).
 *   Menu: 1/2/3 ou setas escolhem a dificuldade; ENTER ou clique em
 *         "Comecar" inicia. Clique em um cartão também escolhe.
 *   Jogo: números = ingredientes, Z/Backspace = desfazer, Espaço = servir.
 *   Fim:  ENTER = jogar de novo, Backspace = voltar ao menu.
 *   Qualquer tela: M liga/desliga o som.
 * [v2] recebe a partida (para saber quantos ingredientes existem) e a
 * dificuldade selecionada no menu (para as setas). 'p' pode ser NULL no menu. */
Acao ui_ler_acao(Tela tela, const Partida *p, Dificuldade selecionada);

/* [v3] Carrega a fonte do jogo (assets/Fredoka-Bold.ttf). Chamar logo
 * depois de InitWindow; se o arquivo não existir, usa a fonte padrão.
 * ui_descarregar_fonte libera a fonte (chamar antes de CloseWindow). */
void ui_carregar_fonte(void);
void ui_descarregar_fonte(void);

/* Desenha a tela inteira. 'p' pode ser NULL na TELA_MENU.
 * [v2] 'selecionada' destaca a dificuldade no menu; 'som_ligado' aparece
 * na tela. */
void ui_desenhar(Tela tela, const Partida *p, Dificuldade selecionada, bool som_ligado);

#endif /* UI_H */
