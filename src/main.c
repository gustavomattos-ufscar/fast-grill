/*
 * main.c — Ponto de entrada do jogo com janela (raylib).
 *
 * Responsabilidades:
 *   1) abrir e fechar a janela (e o áudio);
 *   2) rodar o LAÇO PRINCIPAL (game loop), cerca de 60 vezes por segundo:
 *        ler a ação do jogador (ui) -> aplicar no núcleo (jogo) ->
 *        avançar o relógio -> tocar os sons -> desenhar a tela (ui);
 *   3) chamar jogo_tick a cada 1 segundo de tempo real;
 *   4) trocar de tela (menu -> jogo -> fim -> jogo ou menu...).
 *
 * Modo extra para o relatório:
 *   fast-grill --captura   -> gera as imagens captura_menu.png,
 *   captura_jogo.png, captura_erro.png, captura_fim.png e
 *   captura_dificil.png e fecha sozinho.
 *
 * [v2] MUDANÇAS DA VERSÃO 2:
 *   - guarda a dificuldade escolhida no menu e a passa para jogo_iniciar;
 *   - inicia/encerra o áudio (som.c) e toca os sons a partir dos Eventos
 *     anotados pelo núcleo (função tocar_eventos);
 *   - tecla M liga/desliga o som; na tela de fim, Backspace volta ao menu;
 *   - o modo --captura também gera uma imagem do modo Difícil.
 */

#include <string.h>   /* strcmp */
#include <time.h>     /* time */
#include "raylib.h"
#include "jogo.h"
#include "ui.h"
#include "som.h"      /* [v2] */

#define SEGUNDOS_POR_TICK 1.0f   /* 1 tick do jogo = 1 segundo real */

/* Traduz a ação lida pela interface em uma chamada ao núcleo.
 * É AQUI que a interface e as regras se encontram. */
static void aplicar_acao(Partida *p, Acao acao) {
    switch (acao.tipo) {
        case ACAO_EMPILHAR: jogo_empilhar(p, acao.ingrediente); break;
        case ACAO_DESFAZER: jogo_desfazer(p);                   break;
        case ACAO_SERVIR:   jogo_servir(p);                     break;
        default:            break;   /* nenhuma ação de jogo neste quadro */
    }
}

/* [v2] Olha o que o núcleo anotou (Eventos) e toca o som de cada coisa.
 * Alguns sons "ganham" de outros: se a partida acabou, toca só o som de
 * fim (e não o de vida perdida); se subiu de nível, o som de nível
 * substitui o de pedido servido. */
static void tocar_eventos(const Eventos *e) {
    if (e->terminou) {
        som_tocar(SOM_FIM);
    } else if (e->perdeu_vida) {
        som_tocar(SOM_VIDA);
    }
    if (e->subiu_nivel) {
        som_tocar(SOM_NIVEL);
    } else if (e->serviu) {
        som_tocar(SOM_SERVIR);
    }
    if (e->errou)          som_tocar(SOM_ERRO);
    if (e->empilhou)       som_tocar(SOM_EMPILHAR);
    if (e->desfez)         som_tocar(SOM_DESFAZER);
    if (e->cliente_chegou) som_tocar(SOM_CHEGADA);
}

/* ------------------------------------------------------------------------
 * Modo --captura: joga sozinho alguns passos fixos (semente 42) e salva
 * imagens da tela. Serve como evidência no relatório e para conferir a
 * interface sem precisar jogar. (Sem som: o áudio nem é ligado.)
 * ------------------------------------------------------------------------ */
static void desenhar_e_salvar(Tela tela, const Partida *p, Dificuldade sel, const char *arquivo) {
    int i;
    /* Desenha alguns quadros iguais para a imagem estar estável na tela. */
    for (i = 0; i < 3; i++) {
        BeginDrawing();
        ui_desenhar(tela, p, sel, true);
        EndDrawing();
    }
    TakeScreenshot(arquivo);   /* salva na pasta onde o jogo foi aberto */
}

/* Monta o resto da receita da frente (a partir dos acertos) e serve. */
static void completar_e_servir(Partida *p) {
    Pedido frente;
    int j;
    if (!fila_frente(p->fila, &frente)) {
        return;
    }
    while (pilha_tamanho(p->prato) > p->acertos) {
        jogo_desfazer(p);                      /* tira o que estiver errado */
    }
    for (j = p->acertos; j < frente.receita.tamanho; j++) {
        jogo_empilhar(p, frente.receita.itens[j]);
    }
    jogo_servir(p);
}

static int gerar_capturas(void) {
    Partida p;
    Pedido frente;
    int i;

    InitWindow(LARGURA_TELA, ALTURA_TELA, "Fast Grill - capturas");
    ui_carregar_fonte();             /* [v3] */

    /* [v2] Menu com o Médio selecionado. */
    desenhar_e_salvar(TELA_MENU, NULL, DIFICULDADE_MEDIO, "captura_menu.png");

    /* Partida no Fácil (igual à versão 1). */
    if (!jogo_iniciar(&p, 42, DIFICULDADE_FACIL)) {
        CloseWindow();
        return 1;
    }
    /* Passa 3 ticks e monta os 2 primeiros ingredientes certos. */
    for (i = 0; i < 3; i++) jogo_tick(&p);
    fila_frente(p.fila, &frente);
    for (i = 0; i < 2 && i < frente.receita.tamanho; i++) {
        jogo_empilhar(&p, frente.receita.itens[i]);
    }
    desenhar_e_salvar(TELA_JOGO, &p, p.dificuldade, "captura_jogo.png");

    /* Coloca um ingrediente errado e tenta servir. */
    jogo_empilhar(&p, frente.receita.itens[1] == ALFACE ? TOMATE : ALFACE);
    jogo_servir(&p);
    desenhar_e_salvar(TELA_JOGO, &p, p.dificuldade, "captura_erro.png");

    /* Corrige, serve, atende mais alguns e depois deixa o tempo correr. */
    for (i = 0; i < 4; i++) {
        completar_e_servir(&p);
        jogo_tick(&p);
    }
    while (!jogo_terminou(&p)) jogo_tick(&p);
    desenhar_e_salvar(TELA_FIM, &p, p.dificuldade, "captura_fim.png");
    jogo_destruir(&p);

    /* [v2] Partida no Difícil: 8 ingredientes, fila de 4 e penalidade. */
    if (!jogo_iniciar(&p, 42, DIFICULDADE_DIFICIL)) {
        CloseWindow();
        return 1;
    }
    for (i = 0; i < 6; i++) jogo_tick(&p);
    fila_frente(p.fila, &frente);
    for (i = 0; i < frente.receita.tamanho - 1; i++) {   /* falta o último */
        jogo_empilhar(&p, frente.receita.itens[i]);
    }
    jogo_servir(&p);   /* incompleto de propósito: mostra a penalidade */
    desenhar_e_salvar(TELA_JOGO, &p, p.dificuldade, "captura_dificil.png");
    jogo_destruir(&p);

    ui_descarregar_fonte();          /* [v3] */
    CloseWindow();
    return 0;
}

/* ------------------------------------------------------------------------ */

/* Começa uma partida nova (liberando a anterior, se existir). */
static bool comecar_partida(Partida *p, bool *existe_partida, Dificuldade d) {
    if (*existe_partida) {
        jogo_destruir(p);
    }
    *existe_partida = jogo_iniciar(p, (unsigned int)time(NULL), d);
    return *existe_partida;
}

int main(int argc, char *argv[]) {
    SetTraceLogLevel(LOG_WARNING);   /* raylib: mostra só avisos e erros */

    if (argc > 1 && strcmp(argv[1], "--captura") == 0) {
        return gerar_capturas();
    }

    InitWindow(LARGURA_TELA, ALTURA_TELA, "Fast Grill");
    SetTargetFPS(60);                /* no máximo 60 quadros por segundo */
    ui_carregar_fonte();             /* [v3] fonte Fredoka (assets/) */
    som_iniciar();                   /* [v2] liga o áudio e gera os sons */

    Partida partida;
    bool existe_partida = false;     /* já chamamos jogo_iniciar? */
    Tela tela = TELA_MENU;
    Dificuldade escolhida = DIFICULDADE_FACIL;   /* [v2] seleção do menu */
    float acumulado = 0.0f;          /* tempo real juntado desde o último tick */

    /* LAÇO PRINCIPAL: cada volta é um quadro (frame). Termina quando o
     * jogador fecha a janela ou aperta ESC. */
    while (!WindowShouldClose()) {
        Acao acao = ui_ler_acao(tela, existe_partida ? &partida : NULL, escolhida);

        if (acao.tipo == ACAO_ALTERNAR_SOM) {          /* [v2] tecla M */
            som_alternar();
            som_tocar(SOM_MENU);
        }

        if (tela == TELA_MENU) {
            /* [v2] Menu: escolher a dificuldade e começar. */
            if (acao.tipo == ACAO_ESCOLHER_DIFICULDADE) {
                escolhida = acao.dificuldade;
                som_tocar(SOM_MENU);
            } else if (acao.tipo == ACAO_CONFIRMAR) {
                if (!comecar_partida(&partida, &existe_partida, escolhida)) {
                    break;               /* sem memória: encerra */
                }
                acumulado = 0.0f;
                tela = TELA_JOGO;
                som_tocar(SOM_INICIAR);
            }
        } else if (tela == TELA_JOGO) {
            aplicar_acao(&partida, acao);

            /* Relógio: GetFrameTime() diz quantos segundos durou o último
             * quadro (~0,016 s a 60 FPS). Somamos até dar 1 segundo e aí
             * chamamos jogo_tick. O 'while' garante que, se o computador
             * travar por 2 s, os 2 ticks atrasados acontecem. */
            acumulado += GetFrameTime();
            while (acumulado >= SEGUNDOS_POR_TICK) {
                jogo_tick(&partida);
                acumulado -= SEGUNDOS_POR_TICK;
            }

            /* [v2] Sons do que aconteceu neste quadro; depois, limpa. */
            tocar_eventos(&partida.eventos);
            jogo_limpar_eventos(&partida);

            if (jogo_terminou(&partida)) {
                tela = TELA_FIM;
            }
        } else {                          /* TELA_FIM */
            if (acao.tipo == ACAO_CONFIRMAR) {
                /* Jogar de novo na MESMA dificuldade. */
                if (!comecar_partida(&partida, &existe_partida, escolhida)) {
                    break;
                }
                acumulado = 0.0f;
                tela = TELA_JOGO;
                som_tocar(SOM_INICIAR);
            } else if (acao.tipo == ACAO_VOLTAR_MENU) {   /* [v2] */
                tela = TELA_MENU;
                som_tocar(SOM_MENU);
            }
        }

        BeginDrawing();
        ui_desenhar(tela, existe_partida ? &partida : NULL, escolhida, som_esta_ligado());
        EndDrawing();
    }

    if (existe_partida) {
        jogo_destruir(&partida);
    }
    som_encerrar();                  /* [v2] */
    ui_descarregar_fonte();          /* [v3] */
    CloseWindow();
    return 0;
}
