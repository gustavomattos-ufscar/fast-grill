/*
 * jogo.c — Implementação das regras do Fast Grill (ver jogo.h).
 *
 * =====================================================================
 *  A IDEIA DO "CONTADOR DE ACERTOS" (decisão da Fase 1)
 * =====================================================================
 * Para saber se o prato está certo, o jeito "ingênuo" seria percorrer a
 * pilha inteira e comparar com a receita. Mas a pilha só deixa ver o
 * TOPO: percorrer tudo descaracteriza o TAD.
 *
 * Então guardamos um número, 'acertos' = quantos ingredientes, a partir
 * da BASE do prato, estão iguais à receita, sem nenhum erro no meio.
 *
 *   Receita: Pao, Carne, Queijo, Pao
 *   Prato (base->topo)   acertos   por quê
 *   Pao                  1         Pao == receita[0]
 *   Pao, Carne           2         Carne == receita[1]
 *   Pao, Carne, Alface   2         Alface != receita[2] (errou)
 *   Pao, Carne, Alface, Queijo  2  depois de um erro, nada mais conta
 *
 * Regras de atualização (cada uma custa O(1)):
 *   - EMPILHAR x: se o prato estava todo certo (acertos == tamanho antes)
 *     e x é o próximo da receita (receita[acertos] == x), acertos++.
 *   - DESFAZER: se o prato estava todo certo (acertos == tamanho antes),
 *     o ingrediente removido era um acerto, então acertos--. Se não, o
 *     removido era um ingrediente errado e acertos não muda.
 *   - SERVIR é válido quando acertos == tamanho do prato == tamanho da
 *     receita (tudo certo, nada sobrando, nada faltando).
 *
 * Como o contador vale só para a receita do pedido da FRENTE, sempre que
 * a frente muda (servido ou desistência) o prato é esvaziado.
 *
 * [v2] Novidades desta versão (procure por "[v2]" no arquivo):
 *   - tabela CONFIGS com os números de cada dificuldade;
 *   - penalidade de paciência ao servir errado (Médio e Difícil);
 *   - ingrediente fora da dificuldade é recusado;
 *   - marcação de Eventos para os efeitos sonoros.
 */

#include <stdio.h>    /* snprintf */
#include "jogo.h"

/* ------------------------------------------------------------------------
 * [v2] TABELA DE DIFICULDADES
 *
 * Cada linha é uma ConfigDificuldade, na ordem dos campos da struct:
 *   nome, ingredientes, paciência base, paciência por ingrediente,
 *   pontos por ingrediente, intervalo inicial, intervalo mínimo,
 *   tamanho inicial da receita, pedidos por nível, fila máxima,
 *   penalidade por erro.
 *
 * O FÁCIL é exatamente o jogo da versão 1. O que fica mais difícil:
 *   - mais ingredientes (mais botões para escolher, receitas novas);
 *   - menos paciência por cliente;
 *   - clientes chegando mais rápido, e acelerando mais cedo;
 *   - receitas grandes desde o começo;
 *   - no Difícil a fila lota com 4 (não 5);
 *   - servir errado faz o cliente perder paciência.
 * Em troca, cada ingrediente vale mais pontos.
 * ------------------------------------------------------------------------ */
static const ConfigDificuldade CONFIGS[NUM_DIFICULDADES] = {
    /*  nome      ingr  pac.base pac/ingr pts/ingr interv.ini interv.min tam.ini ped/nivel fila penal */
    { "Facil",     5,     8,       3,      10,       10,        4,         4,      5,       5,    0 },
    { "Medio",     7,     6,       2,      15,        8,        3,         5,      4,       5,    2 },
    { "Dificil",   8,     5,       2,      20,        6,        3,         6,      3,       4,    3 }
};

ConfigDificuldade jogo_config(Dificuldade d) {
    if ((int)d < 0 || (int)d >= NUM_DIFICULDADES) {
        return CONFIGS[DIFICULDADE_FACIL];   /* valor inválido: usa o Fácil */
    }
    return CONFIGS[d];
}

/* ------------------------------------------------------------------------
 * Funções auxiliares internas ('static' = só existem dentro deste arquivo)
 * ------------------------------------------------------------------------ */

/* Escreve um aviso fixo em p->mensagem. snprintf com "%s" copia o texto
 * sem passar do tamanho do vetor (nunca estoura a memória). */
static void avisar(Partida *p, const char *texto) {
    snprintf(p->mensagem, TAM_MENSAGEM, "%s", texto);
}

/* [v2] Monta as regras de pedido a partir da dificuldade e do nível. */
static RegrasPedido regras_atuais(const Partida *p) {
    RegrasPedido r;
    r.tamanho_max = jogo_tamanho_receita_max(p);
    r.ingredientes_disponiveis = p->config.ingredientes_disponiveis;
    r.paciencia_base = p->config.paciencia_base;
    r.paciencia_por_ingrediente = p->config.paciencia_por_ingrediente;
    r.pontos_por_ingrediente = p->config.pontos_por_ingrediente;
    return r;
}

/* Esvazia o prato usando SÓ a operação desempilhar (respeita o TAD).
 * Custo O(n), com n <= 7. Zera o contador de acertos. */
static void descartar_prato(Partida *p) {
    while (pilha_desempilhar(p->prato, NULL)) {
        /* nada: cada volta do while remove um ingrediente do topo */
    }
    p->acertos = 0;
}

/* Depois que a frente da fila muda, o "relógio" passa a ser o do novo
 * cliente da frente (com a paciência cheia dele). Fila vazia -> 0. */
static void atualizar_paciencia_frente(Partida *p) {
    Pedido frente;
    if (fila_frente(p->fila, &frente)) {
        p->paciencia_frente = frente.paciencia_inicial;
    } else {
        p->paciencia_frente = 0;
    }
}

/* Tira uma vida; se acabarem, marca o fim da partida. */
static void perder_vida(Partida *p) {
    p->vidas--;
    p->perdidos++;
    p->eventos.perdeu_vida = true;              /* [v2] som de vida perdida */
    if (p->vidas <= 0) {
        p->vidas = 0;
        p->terminou = true;
        p->eventos.terminou = true;             /* [v2] som de fim de jogo */
    }
}

/* [v2] Servir errado no Médio/Difícil: o cliente da frente perde
 * 'penalidade_erro' de paciência. Nunca deixa abaixo de 1: a desistência
 * só acontece dentro do jogo_tick (um lugar só cuida disso). */
static void aplicar_penalidade(Partida *p) {
    if (p->config.penalidade_erro <= 0) {
        return;                                 /* Fácil: sem penalidade */
    }
    p->paciencia_frente -= p->config.penalidade_erro;
    if (p->paciencia_frente < 1) {
        p->paciencia_frente = 1;
    }
}

/* Um cliente novo chega. Se a fila estiver lotada, ele vai embora e o
 * jogador perde 1 vida.
 * O limite de pedidos na fila é REGRA DO JOGO e é conferido aqui no
 * núcleo, e não só pela fila_cheia: assim, se na Fase 3 a fila virar
 * encadeada (sem capacidade fixa), o comportamento do jogo continua igual.
 * [v2] o limite vem da dificuldade (config.max_pedidos_fila). */
static void chegar_cliente(Partida *p) {
    RegrasPedido regras = regras_atuais(p);
    Pedido novo = pedido_gerar(p->proximo_numero, &regras, &p->semente);
    p->proximo_numero++;

    if (fila_tamanho(p->fila) >= p->config.max_pedidos_fila || fila_cheia(p->fila)) {
        perder_vida(p);
        snprintf(p->mensagem, TAM_MENSAGEM,
                 "Fila lotada! Cliente #%d foi embora (-1 vida)", novo.numero);
        return;
    }

    bool estava_vazia = fila_vazia(p->fila);
    fila_enfileirar(p->fila, novo);           /* entra no FIM da fila */
    p->eventos.cliente_chegou = true;         /* [v2] som de campainha */

    /* Se a fila estava vazia, este cliente já é a frente: o relógio dele
     * começa agora. Se não estava, ele espera sem perder paciência. */
    if (estava_vazia) {
        atualizar_paciencia_frente(p);
    }
}

/* Chamada depois de cada pedido servido: a cada config.pedidos_por_nivel
 * servidos, sobe de nível -> clientes chegam mais rápido e receitas
 * maiores podem aparecer. */
static void verificar_progressao(Partida *p) {
    if (p->servidos % p->config.pedidos_por_nivel == 0) {
        p->nivel++;
        p->eventos.subiu_nivel = true;          /* [v2] som de nível */
        if (p->intervalo_chegada > p->config.intervalo_minimo) {
            p->intervalo_chegada--;
        }
    }
}

/* ------------------------------------------------------------------------
 * Operações públicas
 * ------------------------------------------------------------------------ */

int jogo_tamanho_receita_max(const Partida *p) {
    int tamanho = p->config.tamanho_receita_inicial + p->nivel;   /* [v2] */
    if (tamanho > RECEITA_MAX) {
        tamanho = RECEITA_MAX;
    }
    return tamanho;
}

void jogo_limpar_eventos(Partida *p) {
    p->eventos.empilhou = false;
    p->eventos.desfez = false;
    p->eventos.serviu = false;
    p->eventos.errou = false;
    p->eventos.cliente_chegou = false;
    p->eventos.perdeu_vida = false;
    p->eventos.subiu_nivel = false;
    p->eventos.terminou = false;
}

bool jogo_iniciar(Partida *p, unsigned int semente, Dificuldade dificuldade) {
    int i;

    /* [v2] Guarda a dificuldade e copia os números dela para a partida. */
    p->dificuldade = dificuldade;
    p->config = jogo_config(dificuldade);

    /* Cria os dois TADs. Se algum falhar, desfaz o que já foi criado.
     * [v2] a capacidade da fila vem da dificuldade (5 ou 4). */
    p->fila = fila_criar(p->config.max_pedidos_fila);
    p->prato = pilha_criar(MAX_PRATO);
    if (p->fila == NULL || p->prato == NULL) {
        fila_destruir(p->fila);     /* as duas aceitam NULL */
        pilha_destruir(p->prato);
        p->fila = NULL;
        p->prato = NULL;
        return false;
    }

    /* Valores iniciais de todos os campos. */
    p->acertos = 0;
    p->paciencia_frente = 0;
    p->tick = 0;
    p->intervalo_chegada = p->config.intervalo_inicial;    /* [v2] */
    p->ticks_para_chegada = p->config.intervalo_inicial;   /* [v2] */
    p->nivel = 0;
    p->pontos = 0;
    p->vidas = VIDAS_INICIAIS;
    p->servidos = 0;
    p->perdidos = 0;
    p->proximo_numero = 1;
    p->semente = semente;
    p->terminou = false;

    /* A partida já começa com alguns clientes na fila. */
    for (i = 0; i < PEDIDOS_INICIAIS; i++) {
        chegar_cliente(p);
    }
    jogo_limpar_eventos(p);   /* [v2] a chegada inicial não toca som */
    avisar(p, "Monte o lanche do cliente da frente!");
    return true;
}

void jogo_destruir(Partida *p) {
    fila_destruir(p->fila);
    pilha_destruir(p->prato);
    p->fila = NULL;     /* boa prática: ponteiro liberado vira NULL */
    p->prato = NULL;
}

bool jogo_terminou(const Partida *p) {
    return p->terminou;
}

void jogo_tick(Partida *p) {
    if (p->terminou) {
        return;
    }
    p->tick++;

    /* --- 1) Paciência: só o cliente da FRENTE perde paciência. ---------
     * (Se todos perdessem ao mesmo tempo, um cliente do MEIO poderia
     * desistir, e tirar alguém do meio não é operação de fila.) */
    if (!fila_vazia(p->fila)) {
        p->paciencia_frente--;
        if (p->paciencia_frente <= 0) {
            Pedido desistiu;
            fila_desenfileirar(p->fila, &desistiu);   /* sai pela FRENTE */
            descartar_prato(p);       /* o prato era para ele: vai fora */
            perder_vida(p);
            snprintf(p->mensagem, TAM_MENSAGEM,
                     "Cliente #%d cansou de esperar e foi embora (-1 vida)",
                     desistiu.numero);
            atualizar_paciencia_frente(p);            /* novo da frente */
            if (p->terminou) {
                return;
            }
        }
    }

    /* --- 2) Chegada de clientes, a cada 'intervalo_chegada' ticks. ------ */
    p->ticks_para_chegada--;
    if (p->ticks_para_chegada <= 0) {
        chegar_cliente(p);
        p->ticks_para_chegada = p->intervalo_chegada;
    }
}

bool jogo_empilhar(Partida *p, Ingrediente ing) {
    Pedido frente;

    if (p->terminou) {
        return false;
    }
    /* [v2] No Fácil só existem 5 ingredientes: Bacon/Ovo/Cebola não valem. */
    if ((int)ing < 0 || (int)ing >= p->config.ingredientes_disponiveis) {
        avisar(p, "Esse ingrediente nao existe nesta dificuldade.");
        p->eventos.errou = true;
        return false;
    }
    /* O prato é sempre montado para o cliente da FRENTE. Sem cliente,
     * não há receita para conferir. */
    if (!fila_frente(p->fila, &frente)) {
        avisar(p, "Nenhum cliente na fila. Aguarde o proximo!");
        p->eventos.errou = true;                       /* [v2] */
        return false;
    }
    /* Limite do prato: regra do jogo (conferida aqui) e do TAD. */
    int tamanho_antes = pilha_tamanho(p->prato);
    if (tamanho_antes >= MAX_PRATO || !pilha_empilhar(p->prato, ing)) {
        avisar(p, "O prato esta cheio! Use desfazer.");
        p->eventos.errou = true;                       /* [v2] */
        return false;
    }

    /* Atualiza o contador de acertos (explicação no topo do arquivo). */
    if (p->acertos == tamanho_antes &&
        p->acertos < frente.receita.tamanho &&
        frente.receita.itens[p->acertos] == ing) {
        p->acertos++;
    }

    p->eventos.empilhou = true;                        /* [v2] */
    snprintf(p->mensagem, TAM_MENSAGEM, "%s no prato.", ingrediente_nome(ing));
    return true;
}

bool jogo_desfazer(Partida *p) {
    Ingrediente removido;

    if (p->terminou) {
        return false;
    }
    int tamanho_antes = pilha_tamanho(p->prato);
    if (!pilha_desempilhar(p->prato, &removido)) {   /* tira só o TOPO */
        avisar(p, "O prato ja esta vazio.");
        p->eventos.errou = true;                       /* [v2] */
        return false;
    }

    /* Se o prato inteiro estava certo, o removido era um acerto. */
    if (p->acertos == tamanho_antes) {
        p->acertos--;
    }

    p->eventos.desfez = true;                          /* [v2] */
    snprintf(p->mensagem, TAM_MENSAGEM, "%s retirado do prato.",
             ingrediente_nome(removido));
    return true;
}

bool jogo_servir(Partida *p) {
    Pedido frente;

    if (p->terminou) {
        return false;
    }
    if (!fila_frente(p->fila, &frente)) {
        avisar(p, "Nenhum cliente para servir.");
        p->eventos.errou = true;                       /* [v2] */
        return false;
    }

    int tamanho_prato = pilha_tamanho(p->prato);
    int tamanho_receita = frente.receita.tamanho;
    int penal = p->config.penalidade_erro;             /* [v2] */

    /* Prato certo = tudo que está nele bate com a receita (acertos ==
     * tamanho do prato) e não falta nada (tamanho do prato == receita). */
    if (p->acertos != tamanho_prato) {
        aplicar_penalidade(p);                         /* [v2] */
        p->eventos.errou = true;                       /* [v2] */
        if (penal > 0) {
            snprintf(p->mensagem, TAM_MENSAGEM,
                     "Prato errado! Cliente impaciente (-%d). Desfaca ate o erro.", penal);
        } else {
            avisar(p, "Prato errado! Desfaca ate o erro e monte de novo.");
        }
        return false;
    }
    if (tamanho_prato < tamanho_receita) {
        aplicar_penalidade(p);                         /* [v2] */
        p->eventos.errou = true;                       /* [v2] */
        if (penal > 0) {
            snprintf(p->mensagem, TAM_MENSAGEM,
                     "Ainda faltam %d ingrediente(s)! Cliente impaciente (-%d).",
                     tamanho_receita - tamanho_prato, penal);
        } else {
            snprintf(p->mensagem, TAM_MENSAGEM,
                     "Ainda faltam %d ingrediente(s)!", tamanho_receita - tamanho_prato);
        }
        return false;
    }
    /* (tamanho_prato > tamanho_receita é impossível aqui: acertos nunca
     * passa do tamanho da receita.) */

    /* Prato correto: entrega! */
    int nivel_antes = p->nivel;
    fila_desenfileirar(p->fila, NULL);   /* cliente atendido sai da FRENTE */
    descartar_prato(p);                  /* prato vazio para o próximo     */
    p->pontos += frente.valor;
    p->servidos++;
    p->eventos.serviu = true;            /* [v2] som de "pedido entregue"  */
    verificar_progressao(p);
    atualizar_paciencia_frente(p);       /* relógio do novo cliente da frente */

    if (p->nivel > nivel_antes) {
        snprintf(p->mensagem, TAM_MENSAGEM,
                 "Pedido #%d servido! +%d pontos. NIVEL %d: mais rapido!",
                 frente.numero, frente.valor, p->nivel + 1);
    } else {
        snprintf(p->mensagem, TAM_MENSAGEM, "Pedido #%d servido! +%d pontos",
                 frente.numero, frente.valor);
    }
    return true;
}
