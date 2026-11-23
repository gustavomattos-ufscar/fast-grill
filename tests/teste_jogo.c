/*
 * teste_jogo.c — Testes de CENÁRIO do núcleo do jogo (regras).
 *
 * Aqui não testamos a fila e a pilha isoladas (isso está em teste_fila.c
 * e teste_pilha.c), e sim as REGRAS: servir, desfazer, contador de
 * acertos, paciência, fila lotada, progressão de nível e fim de partida.
 *
 * Usamos sempre uma semente fixa (42): a sequência de pedidos é sempre a
 * mesma, então os testes dão o mesmo resultado em qualquer computador.
 * Os testes não dependem de QUAL receita foi sorteada: eles leem a
 * receita do pedido da frente e montam (certo ou errado) a partir dela.
 *
 * [v2] MUDANÇAS DA VERSÃO 2:
 *   - jogo_iniciar ganhou o parâmetro de dificuldade: os testes antigos
 *     rodam no FÁCIL (que é o jogo da versão 1);
 *   - os números que eram #define fixos agora são lidos de p.config;
 *   - testes NOVOS: tabela de dificuldades, ingredientes por dificuldade,
 *     penalidade ao servir errado, fila de 4 no Difícil e eventos de som.
 */

#include <stdio.h>
#include <string.h>   /* strstr, strcmp */
#include "teste.h"
#include "../src/jogo.h"

#define SEMENTE 42

/* ------------------------------------------------------------------------
 * Funções de apoio dos testes
 * ------------------------------------------------------------------------ */

/* Devolve o pedido da frente (o teste assume que a fila não está vazia). */
static Pedido frente_de(const Partida *p) {
    Pedido f;
    f.numero = -1;                 /* -1 = "não havia frente" */
    fila_frente(p->fila, &f);
    return f;
}

/* Empilha a receita da frente inteira, na ordem certa. */
static void montar_correto(Partida *p) {
    Pedido f = frente_de(p);
    int i;
    for (i = 0; i < f.receita.tamanho; i++) {
        jogo_empilhar(p, f.receita.itens[i]);
    }
}

/* Avança n ticks. */
static void passar_ticks(Partida *p, int n) {
    int i;
    for (i = 0; i < n; i++) {
        jogo_tick(p);
    }
}

/* Um ingrediente diferente de 'ing' (para montar pratos errados). */
static Ingrediente outro_ingrediente(Ingrediente ing) {
    return (ing == TOMATE) ? PAO : TOMATE;
}

/* [v2] Procura uma receita do cardápio pelo nome. */
static bool buscar_receita(const char *nome, Receita *r) {
    int i;
    for (i = 0; i < cardapio_quantidade(); i++) {
        cardapio_receita(i, r);
        if (strcmp(r->nome, nome) == 0) {
            return true;
        }
    }
    return false;
}

/* [v2] true se a receita usa o ingrediente 'ing'. */
static bool receita_usa(const Receita *r, Ingrediente ing) {
    int i;
    for (i = 0; i < r->tamanho; i++) {
        if (r->itens[i] == ing) return true;
    }
    return false;
}

/* [v2] Regras de pedido "na mão" para os testes do cardápio. */
static RegrasPedido regras(int tamanho_max, int ingredientes) {
    RegrasPedido r = { tamanho_max, ingredientes, 8, 3, 10 };
    return r;
}

/* [v2] true se nenhum evento está ligado. */
static bool sem_eventos(const Eventos *e) {
    return !e->empilhou && !e->desfez && !e->serviu && !e->errou &&
           !e->cliente_chegou && !e->perdeu_vida && !e->subiu_nivel && !e->terminou;
}

/* ------------------------------------------------------------------------ */

static void testar_cardapio(void) {
    int i;
    Receita r;
    secao("Cardapio e geracao de pedidos (pedido.c)");

    bool tamanhos_ok = true;
    for (i = 0; i < cardapio_quantidade(); i++) {
        cardapio_receita(i, &r);
        if (r.tamanho < RECEITA_MIN || r.tamanho > RECEITA_MAX) {
            tamanhos_ok = false;
        }
    }
    verifica(cardapio_quantidade() > 0, "cardapio tem receitas");
    verifica(tamanhos_ok, "todas as receitas tem de 3 a 7 ingredientes");
    verifica(!cardapio_receita(-1, &r), "indice -1 do cardapio e recusado");
    verifica(!cardapio_receita(cardapio_quantidade(), &r), "indice alem do fim e recusado");

    unsigned int s = SEMENTE;
    RegrasPedido facil = regras(4, NUM_INGREDIENTES_BASICOS);
    bool respeita_limite = true;
    bool so_basicos = true;
    for (i = 0; i < 200; i++) {
        Pedido p = pedido_gerar(i, &facil, &s);
        if (p.receita.tamanho > 4) respeita_limite = false;
        if (!receita_disponivel(&p.receita, NUM_INGREDIENTES_BASICOS)) so_basicos = false;
    }
    verifica(respeita_limite, "200 sorteios com limite 4 nunca passam de 4 ingredientes");
    verifica(so_basicos, "[v2] com 5 ingredientes, nunca sai Bacon, Ovo ou Cebola");

    unsigned int s1 = SEMENTE, s2 = SEMENTE;
    RegrasPedido todas = regras(7, NUM_INGREDIENTES);
    bool iguais = true;
    for (i = 0; i < 20; i++) {
        Pedido a = pedido_gerar(i, &todas, &s1);
        Pedido b = pedido_gerar(i, &todas, &s2);
        if (strcmp(a.receita.nome, b.receita.nome) != 0) iguais = false;
    }
    verifica(iguais, "mesma semente gera a mesma sequencia de pedidos");

    cardapio_receita(2, &r);           /* Cheeseburguer, 4 itens */
    Pedido p = pedido_criar(1, r, &facil);
    verifica(p.valor == 10 * r.tamanho, "valor = pontos por ingrediente x tamanho");
    verifica(p.paciencia_inicial == 8 + 3 * r.tamanho,
             "paciencia = base + por ingrediente x tamanho");
}

/* [v2] Ingredientes e receitas novos aparecem só onde devem. */
static void testar_ingredientes_por_dificuldade(void) {
    int i;
    Receita r;
    secao("[v2] Ingredientes e receitas por dificuldade");

    verifica(buscar_receita("X-Bacon", &r), "a receita X-Bacon existe");
    verifica(!receita_disponivel(&r, 5), "X-Bacon NAO esta disponivel com 5 ingredientes (Facil)");
    verifica(receita_disponivel(&r, 7), "X-Bacon esta disponivel com 7 ingredientes (Medio)");
    verifica(buscar_receita("Cebolada", &r), "a receita Cebolada existe");
    verifica(!receita_disponivel(&r, 7), "Cebolada NAO esta disponivel no Medio");
    verifica(receita_disponivel(&r, 8), "Cebolada esta disponivel no Dificil");

    unsigned int s = SEMENTE;
    RegrasPedido medio = regras(7, 7);
    bool sem_cebola = true, apareceu_novo = false;
    for (i = 0; i < 300; i++) {
        Pedido p = pedido_gerar(i, &medio, &s);
        if (receita_usa(&p.receita, CEBOLA)) sem_cebola = false;
        if (receita_usa(&p.receita, BACON) || receita_usa(&p.receita, OVO)) apareceu_novo = true;
    }
    verifica(sem_cebola, "Medio: 300 sorteios sem nenhuma Cebola");
    verifica(apareceu_novo, "Medio: Bacon/Ovo aparecem nos pedidos");

    s = SEMENTE;
    RegrasPedido dificil = regras(7, 8);
    bool apareceu_cebola = false;
    for (i = 0; i < 300; i++) {
        Pedido p = pedido_gerar(i, &dificil, &s);
        if (receita_usa(&p.receita, CEBOLA)) apareceu_cebola = true;
    }
    verifica(apareceu_cebola, "Dificil: Cebola aparece nos pedidos");

    Partida p;
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    verifica(!jogo_empilhar(&p, BACON), "Facil: empilhar Bacon e recusado");
    verifica(pilha_vazia(p.prato), "Facil: o prato continua vazio");
    jogo_destruir(&p);

    jogo_iniciar(&p, SEMENTE, DIFICULDADE_MEDIO);
    verifica(jogo_empilhar(&p, BACON), "Medio: empilhar Bacon funciona");
    verifica(!jogo_empilhar(&p, CEBOLA), "Medio: empilhar Cebola e recusado");
    jogo_destruir(&p);

    jogo_iniciar(&p, SEMENTE, DIFICULDADE_DIFICIL);
    verifica(jogo_empilhar(&p, CEBOLA), "Dificil: empilhar Cebola funciona");
    jogo_destruir(&p);
}

/* [v2] A tabela de dificuldades: o Fácil é a v1 e cada nível aperta mais. */
static void testar_configuracoes(void) {
    int d;
    secao("[v2] Tabela de dificuldades");

    ConfigDificuldade f = jogo_config(DIFICULDADE_FACIL);
    verifica(f.ingredientes_disponiveis == 5 && f.paciencia_base == 8 &&
             f.paciencia_por_ingrediente == 3 && f.pontos_por_ingrediente == 10,
             "Facil: 5 ingredientes, paciencia 8 + 3/item, 10 pts/item (igual a v1)");
    verifica(f.intervalo_inicial == 10 && f.intervalo_minimo == 4 &&
             f.tamanho_receita_inicial == 4 && f.pedidos_por_nivel == 5 &&
             f.max_pedidos_fila == 5 && f.penalidade_erro == 0,
             "Facil: chegada 10->4, receitas ate 4, 5 por nivel, fila 5, sem penalidade (igual a v1)");

    bool cada_vez_mais_dificil = true;
    for (d = 1; d < NUM_DIFICULDADES; d++) {
        ConfigDificuldade a = jogo_config((Dificuldade)(d - 1));
        ConfigDificuldade b = jogo_config((Dificuldade)d);
        if (b.ingredientes_disponiveis < a.ingredientes_disponiveis ||
            b.paciencia_base > a.paciencia_base ||
            b.paciencia_por_ingrediente > a.paciencia_por_ingrediente ||
            b.intervalo_inicial >= a.intervalo_inicial ||
            b.tamanho_receita_inicial < a.tamanho_receita_inicial ||
            b.pedidos_por_nivel > a.pedidos_por_nivel ||
            b.max_pedidos_fila > a.max_pedidos_fila ||
            b.penalidade_erro < a.penalidade_erro ||
            b.pontos_por_ingrediente <= a.pontos_por_ingrediente) {
            cada_vez_mais_dificil = false;
        }
    }
    verifica(cada_vez_mais_dificil,
             "Facil -> Medio -> Dificil: nenhuma regra fica mais facil (e os pontos sobem)");
    verifica(jogo_config(DIFICULDADE_DIFICIL).max_pedidos_fila == 4, "Dificil: a fila lota com 4");
    verifica(jogo_config((Dificuldade)99).ingredientes_disponiveis == 5,
             "dificuldade invalida usa o Facil");

    Partida p;
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_DIFICIL);
    verifica(p.dificuldade == DIFICULDADE_DIFICIL && p.config.max_pedidos_fila == 4,
             "jogo_iniciar guarda a dificuldade e a configuracao");
    verifica(p.ticks_para_chegada == 6, "Dificil: primeiro cliente novo em 6 ticks");
    verifica(frente_de(&p).paciencia_inicial ==
             5 + 2 * frente_de(&p).receita.tamanho, "Dificil: paciencia = 5 + 2 x tamanho");
    jogo_destruir(&p);
}

static void testar_inicio(void) {
    Partida p;
    secao("Inicio da partida (Facil)");
    verifica(jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL), "jogo_iniciar funciona");
    verifica(p.vidas == 3, "comeca com 3 vidas");
    verifica(p.pontos == 0, "comeca com 0 pontos");
    verifica(fila_tamanho(p.fila) == 2, "comeca com 2 pedidos na fila");
    verifica(pilha_vazia(p.prato), "comeca com o prato vazio");
    verifica(p.acertos == 0, "contador de acertos comeca em 0");
    verifica(p.paciencia_frente == frente_de(&p).paciencia_inicial,
             "relogio da frente comeca com a paciencia cheia do pedido #1");
    verifica(frente_de(&p).numero == 1, "o pedido da frente e o #1");
    verifica(!jogo_terminou(&p), "partida nao comeca terminada");
    jogo_destruir(&p);
}

static void testar_servir_correto(void) {
    Partida p;
    secao("Servir o prato correto");
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    Pedido f = frente_de(&p);

    montar_correto(&p);
    verifica(p.acertos == f.receita.tamanho, "acertos == tamanho da receita");
    verifica(jogo_servir(&p), "servir funciona com o prato igual a receita");
    verifica(p.pontos == f.valor, "ganhou os pontos do pedido (10 por ingrediente)");
    verifica(p.servidos == 1, "1 pedido servido");
    verifica(fila_tamanho(p.fila) == 1, "o pedido saiu da fila (sobrou 1)");
    verifica(frente_de(&p).numero == 2, "agora a frente e o pedido #2 (FIFO)");
    verifica(pilha_vazia(p.prato) && p.acertos == 0, "prato e contador zerados para o proximo");
    verifica(p.paciencia_frente == frente_de(&p).paciencia_inicial,
             "o relogio passou a ser o do novo cliente da frente");
    verifica(strstr(p.mensagem, "servido") != NULL, "mensagem avisa que foi servido");
    jogo_destruir(&p);
}

static void testar_contador_e_desfazer(void) {
    Partida p;
    secao("Contador de acertos + desfazer (LIFO)");
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    Pedido f = frente_de(&p);
    Ingrediente errado = outro_ingrediente(f.receita.itens[1]);

    jogo_empilhar(&p, f.receita.itens[0]);
    verifica(p.acertos == 1, "1o ingrediente certo -> acertos = 1");
    jogo_empilhar(&p, errado);
    verifica(p.acertos == 1 && pilha_tamanho(p.prato) == 2,
             "ingrediente errado entra no prato mas acertos continua 1");
    verifica(!jogo_servir(&p), "servir falha com prato errado");
    verifica(strstr(p.mensagem, "errado") != NULL, "mensagem explica que o prato esta errado");
    verifica(p.pontos == 0 && fila_tamanho(p.fila) == 2, "servir errado nao muda pontos nem fila");

    verifica(jogo_desfazer(&p), "desfazer tira o topo (o errado)");
    verifica(p.acertos == 1 && pilha_tamanho(p.prato) == 1,
             "tirar o errado nao muda acertos (continua 1)");
    verifica(jogo_desfazer(&p), "desfazer de novo tira o 1o ingrediente");
    verifica(p.acertos == 0 && pilha_vazia(p.prato), "tirar um acerto diminui acertos para 0");
    verifica(!jogo_desfazer(&p), "desfazer com prato vazio falha");

    montar_correto(&p);
    verifica(jogo_servir(&p), "depois de corrigir, servir funciona");
    jogo_destruir(&p);
}

static void testar_ordem_errada(void) {
    Partida p;
    int i;
    secao("Mesmos ingredientes em ordem errada");
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    Pedido f = frente_de(&p);

    /* Monta a receita com os dois primeiros trocados de lugar. */
    jogo_empilhar(&p, f.receita.itens[1]);
    jogo_empilhar(&p, f.receita.itens[0]);
    for (i = 2; i < f.receita.tamanho; i++) {
        jogo_empilhar(&p, f.receita.itens[i]);
    }
    verifica(pilha_tamanho(p.prato) == f.receita.tamanho, "prato tem o tamanho da receita");
    verifica(p.acertos == 0, "acertos = 0 (a base ja esta errada)");
    verifica(!jogo_servir(&p), "servir falha: a ordem importa");
    jogo_destruir(&p);
}

static void testar_incompleto_e_cheio(void) {
    Partida p;
    int i;
    secao("Prato incompleto e prato cheio");
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    Pedido f = frente_de(&p);

    for (i = 0; i < f.receita.tamanho - 1; i++) {   /* falta o ultimo */
        jogo_empilhar(&p, f.receita.itens[i]);
    }
    verifica(!jogo_servir(&p), "servir falha com prato incompleto");
    verifica(strstr(p.mensagem, "faltam") != NULL, "mensagem diz que faltam ingredientes");

    while (jogo_desfazer(&p)) { }                    /* esvazia */
    for (i = 0; i < MAX_PRATO; i++) {
        jogo_empilhar(&p, PAO);
    }
    verifica(pilha_tamanho(p.prato) == MAX_PRATO, "prato chega a 7 ingredientes");
    verifica(!jogo_empilhar(&p, CARNE), "o 8o ingrediente e recusado (prato cheio)");
    verifica(pilha_tamanho(p.prato) == MAX_PRATO, "tamanho continua 7");
    jogo_destruir(&p);
}

static void testar_desistencia(void) {
    Partida p;
    secao("Paciencia: cliente da frente desiste");
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    Pedido f = frente_de(&p);
    Pedido segundo;
    fila_consultar(p.fila, 1, &segundo);

    jogo_empilhar(&p, f.receita.itens[0]);           /* comecou a montar */
    passar_ticks(&p, f.paciencia_inicial - 1);
    verifica(p.vidas == 3 && frente_de(&p).numero == 1,
             "1 tick antes do limite o cliente ainda espera");
    verifica(p.paciencia_frente == 1, "falta 1 de paciencia");

    jogo_tick(&p);                                   /* paciencia zera */
    verifica(p.vidas == 2, "perdeu 1 vida");
    verifica(frente_de(&p).numero == 2, "o #1 saiu pela frente; agora a frente e o #2");
    verifica(pilha_vazia(p.prato) && p.acertos == 0, "o prato em montagem foi descartado");
    verifica(p.paciencia_frente == segundo.paciencia_inicial,
             "o #2 comeca com a paciencia cheia (nao perdeu nada esperando)");
    verifica(strstr(p.mensagem, "cansou") != NULL, "mensagem avisa a desistencia");
    jogo_destruir(&p);
}

/* [v2] Servir errado custa paciência no Médio e no Difícil. */
static void testar_penalidade(void) {
    Partida p;
    secao("[v2] Penalidade ao servir errado");

    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    int antes = p.paciencia_frente;
    jogo_servir(&p);                                  /* prato vazio = incompleto */
    verifica(p.paciencia_frente == antes, "Facil: servir errado NAO tira paciencia");
    jogo_destruir(&p);

    jogo_iniciar(&p, SEMENTE, DIFICULDADE_MEDIO);
    antes = p.paciencia_frente;
    jogo_servir(&p);
    verifica(p.paciencia_frente == antes - 2, "Medio: servir incompleto tira 2 de paciencia");
    verifica(strstr(p.mensagem, "impaciente") != NULL, "mensagem avisa a penalidade");
    Pedido f = frente_de(&p);
    jogo_empilhar(&p, outro_ingrediente(f.receita.itens[0]));
    antes = p.paciencia_frente;
    jogo_servir(&p);
    verifica(p.paciencia_frente == antes - 2, "Medio: servir prato errado tira 2 de paciencia");
    jogo_destruir(&p);

    jogo_iniciar(&p, SEMENTE, DIFICULDADE_DIFICIL);
    while (p.paciencia_frente > 2) {
        jogo_tick(&p);
    }
    int numero_frente = frente_de(&p).numero;
    jogo_servir(&p);                                  /* -3, mas para em 1 */
    verifica(p.paciencia_frente == 1, "Dificil: a penalidade para em 1 (nao zera fora do tick)");
    verifica(p.vidas == 3 && frente_de(&p).numero == numero_frente,
             "a penalidade sozinha nao faz o cliente desistir");
    jogo_tick(&p);
    verifica(p.vidas == 2 && frente_de(&p).numero != numero_frente,
             "no tick seguinte o cliente desiste normalmente");
    jogo_destruir(&p);
}

static void testar_fila_vazia(void) {
    Partida p;
    secao("Sem clientes na fila");
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    montar_correto(&p);
    jogo_servir(&p);
    montar_correto(&p);
    jogo_servir(&p);
    verifica(fila_vazia(p.fila), "servindo os 2 iniciais a fila fica vazia");
    verifica(!jogo_empilhar(&p, PAO), "empilhar sem cliente falha");
    verifica(!jogo_servir(&p), "servir sem cliente falha");

    int vidas = p.vidas;
    passar_ticks(&p, p.config.intervalo_inicial);
    verifica(p.vidas == vidas, "esperar com a fila vazia nao tira vida");
    verifica(fila_tamanho(p.fila) == 1, "apos o intervalo de chegada, chega 1 cliente");
    verifica(p.paciencia_frente == frente_de(&p).paciencia_inicial,
             "o relogio do novo cliente comeca cheio");
    jogo_destruir(&p);
}

/* Joga "perfeitamente" ate servir 'meta' pedidos: se ha cliente, monta e
 * serve na hora; se nao ha, espera 1 tick. */
static void servir_ate(Partida *p, int meta) {
    int seguranca = 0;
    while (p->servidos < meta && seguranca < 10000) {
        if (fila_vazia(p->fila)) {
            jogo_tick(p);
        } else {
            montar_correto(p);
            jogo_servir(p);
        }
        seguranca++;
    }
}

static void testar_progressao(void) {
    Partida p;
    secao("Progressao de nivel");
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    verifica(jogo_tamanho_receita_max(&p) == 4, "nivel inicial: receitas de ate 4 itens");

    servir_ate(&p, p.config.pedidos_por_nivel);
    verifica(p.servidos == 5 && p.vidas == 3, "serviu 5 pedidos sem perder vida");
    verifica(p.nivel == 1, "subiu para o nivel 1");
    verifica(p.intervalo_chegada == p.config.intervalo_inicial - 1,
             "intervalo de chegada caiu de 10 para 9");
    verifica(jogo_tamanho_receita_max(&p) == 5, "agora podem vir receitas de 5 itens");

    servir_ate(&p, 30);
    verifica(p.intervalo_chegada == p.config.intervalo_minimo,
             "com 30 servidos o intervalo chega ao minimo (4)");
    verifica(jogo_tamanho_receita_max(&p) == RECEITA_MAX, "receitas de ate 7 itens liberadas");
    jogo_destruir(&p);

    jogo_iniciar(&p, SEMENTE, DIFICULDADE_DIFICIL);              /* [v2] */
    verifica(jogo_tamanho_receita_max(&p) == 6, "[v2] Dificil: ja comeca com receitas de ate 6 itens");
    servir_ate(&p, p.config.pedidos_por_nivel);
    verifica(p.nivel == 1, "[v2] Dificil: sobe de nivel com apenas 3 servidos");
    jogo_destruir(&p);
}

/* Fica sem atender e observa tick a tick ate ver uma fila lotada.
 * Devolve true se viu; '*passou_do_limite' diz se a fila excedeu o maximo. */
static bool esperar_fila_lotada(Partida *p, bool *passou_do_limite) {
    int i;
    int limite = p->config.max_pedidos_fila;
    *passou_do_limite = false;
    for (i = 0; i < 300 && !p->terminou; i++) {
        int vidas_antes = p->vidas;
        int tamanho_antes = fila_tamanho(p->fila);
        int frente_antes = frente_de(p).numero;

        jogo_tick(p);

        if (fila_tamanho(p->fila) > limite) {
            *passou_do_limite = true;
        }
        /* Lotada = perdeu vida, a fila ja estava no limite, continua no
         * limite e a frente e a mesma (ninguem desistiu neste tick). */
        if (p->vidas < vidas_antes && tamanho_antes == limite &&
            fila_tamanho(p->fila) == limite && frente_de(p).numero == frente_antes) {
            return true;
        }
    }
    return false;
}

static void testar_fila_lotada_e_fim(void) {
    Partida p;
    int i;
    bool passou;
    secao("Fila lotada e fim de partida");
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    servir_ate(&p, 30);           /* chega rapido no intervalo minimo */

    verifica(esperar_fila_lotada(&p, &passou),
             "cliente chegou com a fila cheia (5) e foi embora: -1 vida");
    verifica(strstr(p.mensagem, "lotada") != NULL, "mensagem avisa a fila lotada");
    verifica(!passou, "a fila nunca passou de 5 pedidos");

    /* Continua esperando ate as vidas acabarem. */
    for (i = 0; i < 1000 && !p.terminou; i++) {
        jogo_tick(&p);
    }
    verifica(jogo_terminou(&p), "sem vidas a partida termina");
    verifica(p.vidas == 0, "vidas = 0 (nunca fica negativo)");
    verifica(p.perdidos == VIDAS_INICIAIS, "3 clientes perdidos = 3 vidas");

    int tick_final = p.tick;
    int pontos_finais = p.pontos;
    jogo_tick(&p);
    verifica(p.tick == tick_final, "depois do fim, tick nao avanca");
    verifica(!jogo_empilhar(&p, PAO) && !jogo_desfazer(&p) && !jogo_servir(&p),
             "depois do fim, as acoes do jogador sao ignoradas");
    verifica(p.pontos == pontos_finais && p.servidos == 30, "pontuacao final preservada");
    jogo_destruir(&p);
    verifica(p.fila == NULL && p.prato == NULL, "jogo_destruir libera e zera os ponteiros");

    /* [v2] No Difícil a fila lota com 4. */
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_DIFICIL);
    verifica(esperar_fila_lotada(&p, &passou),
             "[v2] Dificil: cliente chegou com 4 na fila e foi embora");
    verifica(!passou, "[v2] Dificil: a fila nunca passou de 4 pedidos");
    jogo_destruir(&p);
}

/* [v2] Eventos que o main.c usa para tocar os sons. */
static void testar_eventos(void) {
    Partida p;
    secao("[v2] Eventos (usados pelos efeitos sonoros)");
    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    verifica(sem_eventos(&p.eventos), "a partida comeca sem eventos pendentes");

    Pedido f = frente_de(&p);
    jogo_empilhar(&p, f.receita.itens[0]);
    verifica(p.eventos.empilhou, "empilhar marca 'empilhou'");
    jogo_limpar_eventos(&p);
    verifica(sem_eventos(&p.eventos), "jogo_limpar_eventos zera tudo");

    jogo_desfazer(&p);
    verifica(p.eventos.desfez, "desfazer marca 'desfez'");
    jogo_limpar_eventos(&p);

    jogo_servir(&p);
    verifica(p.eventos.errou && !p.eventos.serviu, "servir errado marca 'errou'");
    jogo_limpar_eventos(&p);

    montar_correto(&p);
    jogo_limpar_eventos(&p);
    jogo_servir(&p);
    verifica(p.eventos.serviu && !p.eventos.errou, "servir certo marca 'serviu'");
    jogo_limpar_eventos(&p);

    int numero = p.proximo_numero;
    while (p.proximo_numero == numero) {
        jogo_tick(&p);
    }
    verifica(p.eventos.cliente_chegou, "chegada de cliente marca 'cliente_chegou'");
    jogo_limpar_eventos(&p);

    while (p.vidas == 3) {
        jogo_tick(&p);
    }
    verifica(p.eventos.perdeu_vida, "desistencia marca 'perdeu_vida'");
    jogo_destruir(&p);

    jogo_iniciar(&p, SEMENTE, DIFICULDADE_FACIL);
    servir_ate(&p, p.config.pedidos_por_nivel - 1);
    jogo_limpar_eventos(&p);
    servir_ate(&p, p.config.pedidos_por_nivel);
    verifica(p.eventos.subiu_nivel, "subir de nivel marca 'subiu_nivel'");
    while (!p.terminou) {
        jogo_tick(&p);
    }
    verifica(p.eventos.terminou, "fim da partida marca 'terminou'");
    jogo_destruir(&p);
}

int main(void) {
    testar_cardapio();
    testar_ingredientes_por_dificuldade();   /* [v2] */
    testar_configuracoes();                  /* [v2] */
    testar_inicio();
    testar_servir_correto();
    testar_contador_e_desfazer();
    testar_ordem_errada();
    testar_incompleto_e_cheio();
    testar_desistencia();
    testar_penalidade();                     /* [v2] */
    testar_fila_vazia();
    testar_progressao();
    testar_fila_lotada_e_fim();
    testar_eventos();                        /* [v2] */
    return resumo_testes("teste_jogo");
}
