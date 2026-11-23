/*
 * ui.c — Desenho das telas com a raylib (biblioteca EXTERNA, não feita
 * pelo grupo) e leitura de teclado/mouse.
 *
 * Funções da raylib usadas aqui (todas "Draw..." desenham na tela):
 *   DrawRectangleRec / DrawRectangleRounded / DrawRectangleLinesEx -> retângulos
 *   DrawCircle / DrawEllipse / DrawTriangle -> formas
 *   DrawTextEx / MeasureTextEx -> textos com a fonte do jogo (ver escrever
 *   e medir, mais abaixo)
 *   TextFormat("Pontos: %d", x) -> monta texto como o printf
 *   IsKeyPressed, IsMouseButtonPressed, GetMousePosition -> entrada
 *   CheckCollisionPointRec -> o mouse está dentro do retângulo?
 *
 * Coordenadas: (0, 0) é o canto SUPERIOR esquerdo; x cresce para a direita
 * e y cresce para BAIXO.
 *
 * IMPORTANTE: este arquivo só LÊ a partida (const Partida *). Para desenhar
 * a fila e o prato inteiros ele usa fila_consultar e pilha_consultar, as
 * consultas somente leitura dos TADs.
 *
 * [v2] MUDANÇAS DA VERSÃO 2 (procure "[v2]" no arquivo):
 *   - menu com 3 cartões de dificuldade + botão "Comecar";
 *   - etiqueta colorida da dificuldade no cabeçalho da partida;
 *   - botões de ingrediente em quantidade variável (5, 7 ou 8);
 *   - cores dos ingredientes novos (Bacon, Ovo, Cebola);
 *   - número de espaços da fila depende da dificuldade (5 ou 4);
 *   - indicador "Som: ligado/desligado (M)".
 *
 * [v3] FONTE: os textos usam a fonte Fredoka (assets/Fredoka-Bold.ttf,
 * licença livre OFL) no lugar da fonte pixelada padrão da raylib. Todo
 * texto passa por escrever() e medir(); se o arquivo da fonte não for
 * encontrado, elas usam a fonte padrão e o jogo funciona do mesmo jeito.
 */

#include <stddef.h>   /* NULL */
#include "raylib.h"
#include "ui.h"

/* ------------------------------------------------------------------------
 * Cores (r, g, b, transparência)
 * ------------------------------------------------------------------------ */
#define COR_FUNDO        (Color){ 250, 241, 227, 255 }
#define COR_MADEIRA      (Color){  92,  58,  38, 255 }
#define COR_AMARELO      (Color){ 255, 196,  60, 255 }
#define COR_TEXTO        (Color){  50,  35,  25, 255 }
#define COR_CARTAO       (Color){ 255, 252, 245, 255 }
#define COR_CARTAO_FRENTE (Color){ 255, 236, 200, 255 }
#define COR_BORDA        (Color){ 200, 180, 160, 255 }
#define COR_DESTAQUE     (Color){ 235, 120,  20, 255 }
#define COR_CERTO        (Color){  50, 160,  70, 255 }
#define COR_ERRADO       (Color){ 210,  50,  50, 255 }

/* ------------------------------------------------------------------------
 * Posições na tela (em pixels)
 * ------------------------------------------------------------------------ */
#define FILA_X 20             /* cartões da fila: coluna da esquerda   */
#define FILA_Y 95
#define CARTAO_L 410          /* largura do cartão                     */
#define CARTAO_A 80           /* altura do cartão                      */
#define CARTAO_PASSO 88       /* distância entre o topo de 2 cartões   */

#define PRATO_CENTRO_X 610    /* prato: centro da coluna do meio       */
#define PRATO_BASE_Y 470      /* y onde a 1a camada encosta no prato   */
#define CAMADA_L 210          /* largura de cada ingrediente no prato  */
#define CAMADA_A 26           /* altura de cada ingrediente            */
#define CAMADA_PASSO 30       /* distância entre 2 camadas             */
#define RECEITA_X 790         /* coluna da receita da frente           */

#define MENSAGEM_Y 536
#define BOTAO_Y 570
#define BOTAO_A 56
/* [v2] Os botões de ingrediente dividem a faixa x = 20..630 entre si
 * (5, 7 ou 8 botões); Desfazer e Servir ficam fixos à direita. */
#define AREA_INGREDIENTES_X 20
#define AREA_INGREDIENTES_L 610
#define ESPACO_ENTRE_BOTOES 8

/* [v2] Menu: cartões de dificuldade e botão de começar. */
#define CARTAO_DIF_Y 300
#define CARTAO_DIF_L 290
#define CARTAO_DIF_A 200
#define CARTAO_DIF_PASSO 320

/* ------------------------------------------------------------------------
 * [v3] Fonte do jogo
 * ------------------------------------------------------------------------ */
#define ARQUIVO_FONTE "assets/Fredoka-Bold.ttf"
#define FONTE_TAMANHO_BASE 64   /* tamanho em que as letras são geradas;
                                   textos menores são reduzidos a partir dele */

static Font fonte;               /* a fonte usada por escrever() e medir() */
static bool fonte_propria = false;

void ui_carregar_fonte(void) {
    int codigos[224];            /* letras de 32 (' ') a 255 (inclui acentos) */
    int i;
    for (i = 0; i < 224; i++) {
        codigos[i] = 32 + i;
    }

    /* Procura a pasta assets/ ao lado do executável e, se não achar, na
     * pasta de onde o jogo foi aberto. */
    const char *caminho = TextFormat("%s%s", GetApplicationDirectory(), ARQUIVO_FONTE);
    if (!FileExists(caminho)) {
        caminho = ARQUIVO_FONTE;
    }

    if (FileExists(caminho)) {
        fonte = LoadFontEx(caminho, FONTE_TAMANHO_BASE, codigos, 224);
        fonte_propria = (fonte.texture.id != GetFontDefault().texture.id);
    }
    if (fonte_propria) {
        /* Mipmaps + filtro trilinear: o texto fica liso tanto grande
         * quanto pequeno. */
        GenTextureMipmaps(&fonte.texture);
        SetTextureFilter(fonte.texture, TEXTURE_FILTER_TRILINEAR);
    } else {
        TraceLog(LOG_WARNING, "Fonte %s nao encontrada: usando a fonte padrao", ARQUIVO_FONTE);
        fonte = GetFontDefault();
    }
}

void ui_descarregar_fonte(void) {
    if (fonte_propria) {
        UnloadFont(fonte);
        fonte_propria = false;
    }
}

/* Espaço extra entre as letras: a fonte padrão precisa de mais. */
static float espacamento(int tamanho) {
    return fonte_propria ? 0.0f : tamanho / 10.0f;
}

/* Igual ao DrawText da raylib, mas com a fonte do jogo. */
static void escrever(const char *texto, int x, int y, int tamanho, Color cor) {
    if (fonte.texture.id == 0) {             /* ui_carregar_fonte não foi chamada */
        fonte = GetFontDefault();
    }
    DrawTextEx(fonte, texto, (Vector2){ (float)x, (float)y }, (float)tamanho,
               espacamento(tamanho), cor);
}

/* Igual ao MeasureText: largura do texto em pixels. */
static int medir(const char *texto, int tamanho) {
    if (fonte.texture.id == 0) {
        fonte = GetFontDefault();
    }
    return (int)MeasureTextEx(fonte, texto, (float)tamanho, espacamento(tamanho)).x;
}

/* ------------------------------------------------------------------------
 * Funções auxiliares de desenho
 * ------------------------------------------------------------------------ */

/* Cor de cada ingrediente. */
static Color cor_ingrediente(Ingrediente ing) {
    switch (ing) {
        case PAO:    return (Color){ 222, 168,  90, 255 };
        case CARNE:  return (Color){ 120,  62,  35, 255 };
        case QUEIJO: return (Color){ 250, 205,  50, 255 };
        case ALFACE: return (Color){ 100, 185,  70, 255 };
        case TOMATE: return (Color){ 220,  60,  50, 255 };
        case BACON:  return (Color){ 165,  55,  60, 255 };   /* [v2] */
        case OVO:    return (Color){ 255, 247, 215, 255 };   /* [v2] */
        case CEBOLA: return (Color){ 190, 150, 215, 255 };   /* [v2] */
    }
    return GRAY;
}

/* Texto branco em ingredientes escuros, escuro nos claros (leitura). */
static Color cor_texto_sobre(Ingrediente ing) {
    if (ing == CARNE || ing == TOMATE || ing == BACON) {     /* [v2] +BACON */
        return RAYWHITE;
    }
    return COR_TEXTO;
}

/* [v2] Cor de cada dificuldade: verde, laranja, vermelho. */
static Color cor_dificuldade(Dificuldade d) {
    switch (d) {
        case DIFICULDADE_FACIL:   return COR_CERTO;
        case DIFICULDADE_MEDIO:   return COR_DESTAQUE;
        case DIFICULDADE_DIFICIL: return COR_ERRADO;
    }
    return GRAY;
}

/* Escreve um texto centralizado horizontalmente na tela. */
static void texto_centralizado(const char *texto, int y, int tamanho, Color cor) {
    int largura = medir(texto, tamanho);
    escrever(texto, (LARGURA_TELA - largura) / 2, y, tamanho, cor);
}

/* Escreve um texto centralizado dentro de um retângulo. */
static void texto_no_retangulo(const char *texto, Rectangle r, int tamanho, Color cor) {
    int largura = medir(texto, tamanho);
    escrever(texto, (int)(r.x + (r.width - largura) / 2),
             (int)(r.y + (r.height - tamanho) / 2), tamanho, cor);
}

/* Desenha um coração: dois círculos em cima e um triângulo embaixo. */
static void desenhar_coracao(int x, int y, Color cor) {
    DrawCircle(x - 6, y, 7, cor);
    DrawCircle(x + 6, y, 7, cor);
    DrawTriangle((Vector2){ x - 13, y + 2 }, (Vector2){ x, y + 16 },
                 (Vector2){ x + 13, y + 2 }, cor);
}

/* Desenha uma camada de ingrediente no prato. */
static void desenhar_camada(Rectangle r, Ingrediente ing) {
    DrawRectangleRounded(r, 0.45f, 6, cor_ingrediente(ing));
    texto_no_retangulo(ingrediente_nome(ing), r, 16, cor_texto_sobre(ing));
}

/* ------------------------------------------------------------------------
 * Posição dos botões e cartões. A MESMA função serve para DESENHAR e para
 * saber se o mouse clicou, então os dois nunca ficam desalinhados.
 * ------------------------------------------------------------------------ */

/* [v2] Botão do ingrediente i quando há n ingredientes: a faixa de 610 px
 * é dividida em n botões iguais (com 8 px de espaço entre eles). */
static Rectangle retangulo_ingrediente(int i, int n) {
    float largura = (AREA_INGREDIENTES_L - (n - 1) * ESPACO_ENTRE_BOTOES) / (float)n;
    return (Rectangle){ AREA_INGREDIENTES_X + i * (largura + ESPACO_ENTRE_BOTOES),
                        BOTAO_Y, largura, BOTAO_A };
}

static Rectangle retangulo_desfazer(void) {
    return (Rectangle){ 640, BOTAO_Y, 160, BOTAO_A };
}

static Rectangle retangulo_servir(void) {
    return (Rectangle){ 810, BOTAO_Y, 170, BOTAO_A };
}

/* [v2] Cartão da dificuldade d no menu (3 lado a lado). */
static Rectangle retangulo_cartao_dificuldade(Dificuldade d) {
    return (Rectangle){ 35 + (int)d * CARTAO_DIF_PASSO, CARTAO_DIF_Y,
                        CARTAO_DIF_L, CARTAO_DIF_A };
}

/* [v2] Botão "Comecar" do menu. */
static Rectangle retangulo_comecar(void) {
    return (Rectangle){ 360, 522, 280, 50 };
}

/* ------------------------------------------------------------------------
 * Leitura de entrada
 * ------------------------------------------------------------------------ */

/* [v2] Entrada do menu: escolher dificuldade e começar. */
static Acao ler_acao_menu(Acao acao, Dificuldade selecionada) {
    int d;

    /* Teclas 1, 2, 3 escolhem direto. */
    for (d = 0; d < NUM_DIFICULDADES; d++) {
        if (IsKeyPressed(KEY_ONE + d) || IsKeyPressed(KEY_KP_1 + d)) {
            acao.tipo = ACAO_ESCOLHER_DIFICULDADE;
            acao.dificuldade = (Dificuldade)d;
            return acao;
        }
    }
    /* Setas andam uma posição (sem passar das pontas). */
    if (IsKeyPressed(KEY_LEFT) && selecionada > DIFICULDADE_FACIL) {
        acao.tipo = ACAO_ESCOLHER_DIFICULDADE;
        acao.dificuldade = (Dificuldade)(selecionada - 1);
        return acao;
    }
    if (IsKeyPressed(KEY_RIGHT) && selecionada < DIFICULDADE_DIFICIL) {
        acao.tipo = ACAO_ESCOLHER_DIFICULDADE;
        acao.dificuldade = (Dificuldade)(selecionada + 1);
        return acao;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        acao.tipo = ACAO_CONFIRMAR;
        return acao;
    }
    /* Mouse: clique em um cartão escolhe; clique em "Comecar" inicia. */
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        for (d = 0; d < NUM_DIFICULDADES; d++) {
            if (CheckCollisionPointRec(mouse, retangulo_cartao_dificuldade((Dificuldade)d))) {
                acao.tipo = ACAO_ESCOLHER_DIFICULDADE;
                acao.dificuldade = (Dificuldade)d;
                return acao;
            }
        }
        if (CheckCollisionPointRec(mouse, retangulo_comecar())) {
            acao.tipo = ACAO_CONFIRMAR;
        }
    }
    return acao;
}

/* Entrada durante a partida. */
static Acao ler_acao_jogo(Acao acao, const Partida *p) {
    int i;
    int n = p->config.ingredientes_disponiveis;   /* [v2] 5, 7 ou 8 */

    /* Teclado: 1 a n (normal ou teclado numérico) escolhem o ingrediente.
     * KEY_ONE, KEY_TWO... são números seguidos, então KEY_ONE + i funciona. */
    for (i = 0; i < n; i++) {
        if (IsKeyPressed(KEY_ONE + i) || IsKeyPressed(KEY_KP_1 + i)) {
            acao.tipo = ACAO_EMPILHAR;
            acao.ingrediente = (Ingrediente)i;
            return acao;
        }
    }
    if (IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_BACKSPACE)) {
        acao.tipo = ACAO_DESFAZER;
        return acao;
    }
    if (IsKeyPressed(KEY_SPACE)) {
        acao.tipo = ACAO_SERVIR;
        return acao;
    }

    /* Mouse: descobre em qual botão (se algum) foi o clique. */
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        for (i = 0; i < n; i++) {
            if (CheckCollisionPointRec(mouse, retangulo_ingrediente(i, n))) {
                acao.tipo = ACAO_EMPILHAR;
                acao.ingrediente = (Ingrediente)i;
                return acao;
            }
        }
        if (CheckCollisionPointRec(mouse, retangulo_desfazer())) {
            acao.tipo = ACAO_DESFAZER;
        } else if (CheckCollisionPointRec(mouse, retangulo_servir())) {
            acao.tipo = ACAO_SERVIR;
        }
    }
    return acao;
}

Acao ui_ler_acao(Tela tela, const Partida *p, Dificuldade selecionada) {
    Acao acao;
    acao.tipo = ACAO_NENHUMA;
    acao.ingrediente = PAO;
    acao.dificuldade = selecionada;

    /* [v2] M liga/desliga o som em qualquer tela. */
    if (IsKeyPressed(KEY_M)) {
        acao.tipo = ACAO_ALTERNAR_SOM;
        return acao;
    }

    if (tela == TELA_MENU || p == NULL) {
        return ler_acao_menu(acao, selecionada);
    }
    if (tela == TELA_FIM) {
        if (IsKeyPressed(KEY_ENTER)) {
            acao.tipo = ACAO_CONFIRMAR;          /* jogar de novo */
        } else if (IsKeyPressed(KEY_BACKSPACE)) {
            acao.tipo = ACAO_VOLTAR_MENU;        /* [v2] trocar dificuldade */
        }
        return acao;
    }
    return ler_acao_jogo(acao, p);
}

/* ------------------------------------------------------------------------
 * Partes da tela de jogo
 * ------------------------------------------------------------------------ */

static void desenhar_cabecalho(const Partida *p) {
    int i;
    DrawRectangle(0, 0, LARGURA_TELA, 60, COR_MADEIRA);
    escrever("FAST GRILL", 20, 14, 34, COR_AMARELO);

    /* [v2] Etiqueta colorida com a dificuldade da partida. */
    int largura_etiqueta = medir(p->config.nome, 18) + 16;
    Rectangle etiqueta = { 238, 18, largura_etiqueta, 26 };
    DrawRectangleRounded(etiqueta, 0.4f, 6, cor_dificuldade(p->dificuldade));
    texto_no_retangulo(p->config.nome, etiqueta, 18, RAYWHITE);

    escrever(TextFormat("Pontos: %d", p->pontos), 345, 20, 22, RAYWHITE);
    escrever(TextFormat("Servidos: %d", p->servidos), 500, 20, 22, RAYWHITE);
    escrever(TextFormat("Nivel: %d", p->nivel + 1), 665, 20, 22, RAYWHITE);
    escrever("Vidas:", 790, 20, 22, RAYWHITE);
    for (i = 0; i < VIDAS_INICIAIS; i++) {
        desenhar_coracao(885 + i * 34, 26, i < p->vidas ? COR_ERRADO : DARKGRAY);
    }
}

/* Um cartão da fila. 'eh_frente' destaca o cliente que pode ser atendido. */
static void desenhar_cartao(Rectangle r, const Pedido *ped, bool eh_frente, const Partida *p) {
    int j;
    DrawRectangleRec(r, eh_frente ? COR_CARTAO_FRENTE : COR_CARTAO);
    DrawRectangleLinesEx(r, eh_frente ? 3 : 1, eh_frente ? COR_DESTAQUE : COR_BORDA);

    escrever(TextFormat("#%d  %s", ped->numero, ped->receita.nome),
             (int)r.x + 10, (int)r.y + 8, 20, COR_TEXTO);
    if (eh_frente) {
        escrever("FRENTE", (int)(r.x + r.width) - 78, (int)r.y + 10, 16, COR_DESTAQUE);
    }

    /* A receita em quadradinhos coloridos, da base (esquerda) ao topo. */
    for (j = 0; j < ped->receita.tamanho; j++) {
        Rectangle q = { r.x + 10 + j * 56, r.y + 34, 52, 24 };
        Ingrediente ing = ped->receita.itens[j];
        DrawRectangleRec(q, cor_ingrediente(ing));
        DrawRectangleLinesEx(q, 1, Fade(COR_TEXTO, 0.15f));   /* [v2] contorno (Ovo é claro) */
        texto_no_retangulo(ingrediente_nome(ing), q, 13, cor_texto_sobre(ing));
    }

    if (eh_frente) {
        /* Barra de paciência: verde -> amarelo -> vermelho. */
        float fracao = (float)p->paciencia_frente / (float)ped->paciencia_inicial;
        Color cor = COR_CERTO;
        if (fracao < 0.5f) cor = ORANGE;
        if (fracao < 0.25f) cor = COR_ERRADO;
        DrawRectangle((int)r.x + 10, (int)r.y + 66, 390, 8, LIGHTGRAY);
        DrawRectangle((int)r.x + 10, (int)r.y + 66, (int)(390 * fracao), 8, cor);
    } else {
        escrever("aguardando...", (int)r.x + 10, (int)r.y + 62, 14, GRAY);
    }
}

static void desenhar_fila(const Partida *p) {
    int i;
    Pedido ped;
    int espacos = p->config.max_pedidos_fila;   /* [v2] 5 ou 4 */

    escrever(TextFormat("FILA DE PEDIDOS  %d/%d", fila_tamanho(p->fila), espacos),
             FILA_X, 70, 20, COR_TEXTO);
    escrever(TextFormat("proximo cliente em %ds", p->ticks_para_chegada),
             FILA_X + 265, 73, 14, GRAY);

    /* Desenha um espaço para cada lugar da fila; os ocupados recebem o
     * cartão do pedido (posição 0 = frente), os outros ficam "livres". */
    for (i = 0; i < espacos; i++) {
        Rectangle r = { FILA_X, FILA_Y + i * CARTAO_PASSO, CARTAO_L, CARTAO_A };
        if (fila_consultar(p->fila, i, &ped)) {
            desenhar_cartao(r, &ped, i == 0, p);
        } else {
            DrawRectangleLinesEx(r, 1, COR_BORDA);
            texto_no_retangulo("livre", r, 16, COR_BORDA);
        }
    }
}

static void desenhar_prato(const Partida *p) {
    int i;
    Ingrediente ing;
    Pedido frente;
    bool tem_frente = fila_frente(p->fila, &frente);
    int tamanho_prato = pilha_tamanho(p->prato);

    escrever("PRATO", PRATO_CENTRO_X - 35, 70, 20, COR_TEXTO);
    escrever("RECEITA DA FRENTE", RECEITA_X, 72, 16, COR_TEXTO);

    /* O prato (duas elipses: borda cinza e centro branco). */
    DrawEllipse(PRATO_CENTRO_X, PRATO_BASE_Y + 14, 150, 24, LIGHTGRAY);
    DrawEllipse(PRATO_CENTRO_X, PRATO_BASE_Y + 12, 128, 16, RAYWHITE);

    /* Camadas do prato, da base (i = 0) para o topo. A bolinha à esquerda
     * mostra o contador de acertos: verde = confere com a receita,
     * vermelho = errado (desfaça até ele). */
    for (i = 0; i < tamanho_prato; i++) {
        pilha_consultar(p->prato, i, &ing);
        Rectangle c = { PRATO_CENTRO_X - CAMADA_L / 2,
                        PRATO_BASE_Y - CAMADA_A - i * CAMADA_PASSO, CAMADA_L, CAMADA_A };
        desenhar_camada(c, ing);
        DrawCircle((int)c.x - 16, (int)(c.y + CAMADA_A / 2), 7,
                   i < p->acertos ? COR_CERTO : COR_ERRADO);
    }
    if (tamanho_prato == 0) {
        escrever("prato vazio", PRATO_CENTRO_X - 50, PRATO_BASE_Y - 30, 18, GRAY);
    }
    escrever(TextFormat("%d/%d ingredientes", tamanho_prato, MAX_PRATO),
             PRATO_CENTRO_X - 70, PRATO_BASE_Y + 42, 16, GRAY);

    /* Receita da frente, alinhada com as camadas do prato (base embaixo).
     * Destaque laranja = próximo ingrediente esperado. */
    if (!tem_frente) {
        escrever("(sem clientes)", RECEITA_X, 120, 16, GRAY);
        return;
    }
    for (i = 0; i < frente.receita.tamanho; i++) {
        Ingrediente esperado = frente.receita.itens[i];
        Rectangle c = { RECEITA_X, PRATO_BASE_Y - CAMADA_A - i * CAMADA_PASSO, 150, CAMADA_A };
        DrawRectangleRec(c, Fade(cor_ingrediente(esperado), 0.35f));
        bool proximo = (i == p->acertos && p->acertos == tamanho_prato);
        DrawRectangleLinesEx(c, proximo ? 3 : 1, proximo ? COR_DESTAQUE : COR_BORDA);
        escrever(TextFormat("%d. %s", i + 1, ingrediente_nome(esperado)),
                 (int)c.x + 8, (int)c.y + 5, 16, COR_TEXTO);
        if (i < p->acertos) {
            escrever("ok", (int)c.x + 120, (int)c.y + 5, 16, COR_CERTO);
        }
    }
}

/* Desenha um botão arredondado com efeito "hover" (mouse em cima). */
static void desenhar_botao(Rectangle r, Color fundo, const char *rotulo, int tamanho, Color texto) {
    if (CheckCollisionPointRec(GetMousePosition(), r)) {
        fundo = Fade(fundo, 0.75f);
    }
    DrawRectangleRounded(r, 0.25f, 6, fundo);
    texto_no_retangulo(rotulo, r, tamanho, texto);
}

/* [v2] Agora desenha n botões de ingrediente (n depende da dificuldade).
 * Cada botão mostra o nome no meio e a tecla (número) no canto. */
static void desenhar_botoes(const Partida *p) {
    int i;
    int n = p->config.ingredientes_disponiveis;

    for (i = 0; i < n; i++) {
        Ingrediente ing = (Ingrediente)i;
        Rectangle r = retangulo_ingrediente(i, n);
        int tamanho = (r.width >= 100) ? 18 : 15;      /* botões estreitos: fonte menor */
        desenhar_botao(r, cor_ingrediente(ing), ingrediente_nome(ing), tamanho,
                       cor_texto_sobre(ing));
        escrever(TextFormat("%d", i + 1), (int)r.x + 6, (int)r.y + 4, 14,
                 Fade(cor_texto_sobre(ing), 0.8f));
    }
    desenhar_botao(retangulo_desfazer(), (Color){ 130, 130, 140, 255 }, "Desfazer (Z)", 18, RAYWHITE);
    desenhar_botao(retangulo_servir(), COR_CERTO, "Servir (Espaco)", 18, RAYWHITE);
}

/* [v2] Indicador do som, no canto direito da linha de mensagem. */
static void desenhar_indicador_som(bool som_ligado, int x, int y) {
    escrever(som_ligado ? "Som: ligado (M)" : "Som: desligado (M)", x, y, 16,
             som_ligado ? GRAY : COR_ERRADO);
}

static void desenhar_jogo(const Partida *p, bool som_ligado) {
    desenhar_cabecalho(p);
    desenhar_fila(p);
    desenhar_prato(p);
    escrever(p->mensagem, FILA_X, MENSAGEM_Y, 20, COR_DESTAQUE);
    desenhar_indicador_som(som_ligado, 830, MENSAGEM_Y + 3);   /* [v2] */
    desenhar_botoes(p);
}

/* ------------------------------------------------------------------------
 * Telas de menu e de fim
 * ------------------------------------------------------------------------ */

/* [v2] Um cartão de dificuldade no menu. Os textos são montados a partir
 * de jogo_config(d), então se alguém mudar a tabela em jogo.c o menu
 * se atualiza sozinho. */
static void desenhar_cartao_dificuldade(Dificuldade d, bool selecionado) {
    ConfigDificuldade c = jogo_config(d);
    Rectangle r = retangulo_cartao_dificuldade(d);
    Color cor = cor_dificuldade(d);
    int x = (int)r.x + 16;
    int y = (int)r.y + 64;

    DrawRectangleRec(r, selecionado ? Fade(cor, 0.15f) : COR_CARTAO);
    DrawRectangleLinesEx(r, selecionado ? 4 : 1, selecionado ? cor : COR_BORDA);
    escrever(TextFormat("[%d]", (int)d + 1), (int)r.x + 10, (int)r.y + 10, 16, GRAY);
    texto_no_retangulo(c.nome, (Rectangle){ r.x, r.y + 12, r.width, 36 }, 32, cor);

    escrever(TextFormat("%d ingredientes", c.ingredientes_disponiveis), x, y, 16, COR_TEXTO);
    escrever(TextFormat("Paciencia: %d + %d por item", c.paciencia_base,
                        c.paciencia_por_ingrediente), x, y + 24, 16, COR_TEXTO);
    escrever(TextFormat("Cliente a cada %ds (min. %ds)", c.intervalo_inicial,
                        c.intervalo_minimo), x, y + 48, 16, COR_TEXTO);
    escrever(TextFormat("Fila lota com %d | %d pts/item", c.max_pedidos_fila,
                        c.pontos_por_ingrediente), x, y + 72, 16, COR_TEXTO);
    if (c.penalidade_erro > 0) {
        escrever(TextFormat("Servir errado: -%d paciencia", c.penalidade_erro),
                 x, y + 96, 16, COR_ERRADO);
    } else {
        escrever("Servir errado: sem castigo", x, y + 96, 16, COR_CERTO);
    }
}

static void desenhar_menu(Dificuldade selecionada, bool som_ligado) {
    int d;

    DrawRectangle(0, 0, LARGURA_TELA, 130, COR_MADEIRA);
    texto_centralizado("FAST GRILL", 24, 64, COR_AMARELO);
    texto_centralizado("Monte os lanches na ordem certa antes que os clientes desistam!",
                       96, 20, RAYWHITE);

    escrever("- Os clientes formam uma FILA: so o da FRENTE pode ser atendido.", 80, 146, 18, COR_TEXTO);
    escrever("- O lanche e uma PILHA: empilhe da base para o topo, na ordem da receita.", 80, 172, 18, COR_TEXTO);
    escrever("- Errou? 'Desfazer' tira so o TOPO. Esperou demais ou fila lotada = -1 vida.", 80, 198, 18, COR_TEXTO);
    escrever("- Teclas: numeros = ingredientes | Z = desfazer | ESPACO = servir | M = som", 80, 224, 18, COR_TEXTO);

    /* [v2] Seletor de dificuldade. */
    texto_centralizado("ESCOLHA A DIFICULDADE", 264, 22, COR_TEXTO);
    for (d = 0; d < NUM_DIFICULDADES; d++) {
        desenhar_cartao_dificuldade((Dificuldade)d, d == (int)selecionada);
    }

    desenhar_botao(retangulo_comecar(), cor_dificuldade(selecionada),
                   TextFormat("COMECAR: %s", jogo_config(selecionada).nome), 22, RAYWHITE);
    texto_centralizado("Escolha com as setas <- -> ou as teclas 1, 2, 3 (ou clique) e aperte ENTER",
                       584, 16, GRAY);
    texto_centralizado(som_ligado ? "Som ligado (M para desligar)" : "Som desligado (M para ligar)",
                       606, 16, som_ligado ? GRAY : COR_ERRADO);
}

static void desenhar_fim(const Partida *p) {
    /* Escurece a partida que ficou ao fundo e desenha um quadro por cima. */
    DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, Fade(BLACK, 0.65f));
    Rectangle quadro = { 230, 130, 540, 360 };
    DrawRectangleRec(quadro, COR_FUNDO);
    DrawRectangleLinesEx(quadro, 4, COR_DESTAQUE);

    texto_centralizado("FIM DE JOGO", 160, 50, COR_ERRADO);
    texto_centralizado(TextFormat("Dificuldade: %s", p->config.nome), 222, 22,
                       cor_dificuldade(p->dificuldade));                 /* [v2] */
    texto_centralizado(TextFormat("Pontuacao final: %d", p->pontos), 258, 30, COR_TEXTO);
    texto_centralizado(TextFormat("Pedidos servidos: %d", p->servidos), 306, 24, COR_TEXTO);
    texto_centralizado(TextFormat("Clientes perdidos: %d", p->perdidos), 340, 24, COR_TEXTO);
    texto_centralizado(TextFormat("Nivel alcancado: %d", p->nivel + 1), 374, 24, COR_TEXTO);
    texto_centralizado("ENTER = jogar de novo", 420, 20, COR_DESTAQUE);
    texto_centralizado("BACKSPACE = trocar dificuldade  |  ESC = sair", 448, 18, COR_DESTAQUE); /* [v2] */
}

/* ------------------------------------------------------------------------ */

void ui_desenhar(Tela tela, const Partida *p, Dificuldade selecionada, bool som_ligado) {
    ClearBackground(COR_FUNDO);

    if (tela == TELA_MENU || p == NULL) {
        desenhar_menu(selecionada, som_ligado);
    } else if (tela == TELA_JOGO) {
        desenhar_jogo(p, som_ligado);
    } else {                       /* TELA_FIM: a partida ao fundo + placar */
        desenhar_jogo(p, som_ligado);
        desenhar_fim(p);
    }
}
