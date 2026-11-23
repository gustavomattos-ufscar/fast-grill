/*
 * main_texto.c — O jogo no TERMINAL, sem raylib.
 *
 * Para que serve?
 *   1) Mostrar que o núcleo (jogo.c + fila + pilha) funciona sozinho,
 *      sem nenhuma interface gráfica (separação interface x regras).
 *   2) Jogar/demonstrar em qualquer computador, mesmo sem a raylib.
 *   3) Gerar evidências para o relatório: dá para mandar uma lista de
 *      comandos por um arquivo e guardar a saída.
 *
 * Diferença para a versão com janela: aqui o tempo NÃO corre sozinho.
 * O jogador digita "t" para passar 1 tick (ou "t 5" para passar 5).
 * Também não há som (o som usa a raylib).
 *
 * Uso:
 *   fast-grill-texto              (semente aleatória, Fácil)
 *   fast-grill-texto 42           (semente fixa: sempre a mesma partida)
 *   fast-grill-texto 42 dificil   [v2] escolhe a dificuldade:
 *                                 facil | medio | dificil (basta a 1a letra)
 *
 * Comandos:
 *   1..N -> empilha o ingrediente N (N = 5, 7 ou 8, conforme a dificuldade)
 *   d -> desfaz (tira o topo do prato)     s -> serve o cliente da frente
 *   t -> passa 1 tick    t N -> passa N ticks
 *   a -> ajuda           q -> sai
 *
 * [v2] MUDANÇAS DA VERSÃO 2: argumento de dificuldade; a ajuda lista os
 * ingredientes da dificuldade escolhida; o tamanho da fila vem da
 * configuração.
 */

#include <stdio.h>
#include <stdlib.h>   /* atoi */
#include <time.h>     /* time */
#include "jogo.h"

/* Imprime uma receita como "Pao > Carne > Queijo > Pao". */
static void imprimir_receita(const Receita *r) {
    int i;
    for (i = 0; i < r->tamanho; i++) {
        if (i > 0) printf(" > ");
        printf("%s", ingrediente_nome(r->itens[i]));
    }
}

/* [v2] A lista de ingredientes depende da dificuldade. */
static void imprimir_ajuda(const Partida *p) {
    int i;
    printf("\nComandos:\n ");
    for (i = 0; i < p->config.ingredientes_disponiveis; i++) {
        printf(" %d %s |", i + 1, ingrediente_nome((Ingrediente)i));
    }
    printf(" -> empilha no prato\n");
    printf("  d desfaz (tira o topo) | s serve | t passa 1 tick | t N passa N ticks\n");
    printf("  a ajuda | q sai\n");
}

/* Mostra todo o estado da partida.
 * Repare que esta função só LÊ a partida (recebe const Partida *) e usa
 * fila_consultar/pilha_consultar, as consultas somente leitura que
 * existem justamente para a interface poder desenhar tudo. */
static void imprimir_estado(const Partida *p) {
    int i;
    Pedido ped;
    Ingrediente ing;

    printf("\n------------------------------------------------------------------\n");
    printf("Tick %d | Vidas %d | Pontos %d | Servidos %d | Nivel %d | Proximo cliente em %d tick(s)\n",
           p->tick, p->vidas, p->pontos, p->servidos, p->nivel + 1, p->ticks_para_chegada);

    printf("FILA (frente -> fim), %d de %d:\n", fila_tamanho(p->fila),
           p->config.max_pedidos_fila);                              /* [v2] */
    if (fila_vazia(p->fila)) {
        printf("  (vazia)\n");
    }
    for (i = 0; i < fila_tamanho(p->fila); i++) {
        fila_consultar(p->fila, i, &ped);
        printf("  #%-3d %-15s [", ped.numero, ped.receita.nome);
        imprimir_receita(&ped.receita);
        printf("]");
        if (i == 0) {
            printf("  <- FRENTE, paciencia %d/%d", p->paciencia_frente, ped.paciencia_inicial);
        }
        printf("\n");
    }

    printf("PRATO (base -> topo): ");
    if (pilha_vazia(p->prato)) {
        printf("(vazio)");
    }
    for (i = 0; i < pilha_tamanho(p->prato); i++) {
        pilha_consultar(p->prato, i, &ing);
        if (i > 0) printf(" > ");
        printf("%s", ingrediente_nome(ing));
    }
    if (fila_frente(p->fila, &ped)) {
        printf("   [%d de %d certos]", p->acertos, ped.receita.tamanho);
    }
    printf("\n>> %s\n", p->mensagem);
}

/* [v2] Converte o texto do argumento ("facil", "m", "Dificil"...) na
 * dificuldade. Olha só a primeira letra; qualquer outra coisa = Fácil. */
static Dificuldade ler_dificuldade(const char *texto) {
    char c = texto[0];
    if (c == 'm' || c == 'M') return DIFICULDADE_MEDIO;
    if (c == 'd' || c == 'D') return DIFICULDADE_DIFICIL;
    return DIFICULDADE_FACIL;
}

int main(int argc, char *argv[]) {
    Partida partida;
    char linha[64];

    /* Semente: a do argumento, se houver; senão, o relógio do computador. */
    unsigned int semente = (unsigned int)time(NULL);
    if (argc > 1) {
        semente = (unsigned int)atoi(argv[1]);
    }
    /* [v2] Dificuldade: 2º argumento (padrão: Fácil). */
    Dificuldade dificuldade = DIFICULDADE_FACIL;
    if (argc > 2) {
        dificuldade = ler_dificuldade(argv[2]);
    }

    if (!jogo_iniciar(&partida, semente, dificuldade)) {
        printf("Erro: memoria insuficiente para iniciar a partida.\n");
        return 1;
    }

    printf("=== FAST GRILL (modo texto) ===  semente %u | dificuldade %s\n",
           semente, partida.config.nome);
    imprimir_ajuda(&partida);
    imprimir_estado(&partida);

    /* Laço principal: lê um comando por linha até 'q', fim da entrada
     * ou fim da partida. */
    while (!jogo_terminou(&partida)) {
        printf("> ");
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            break;                                  /* acabou a entrada */
        }

        char c = linha[0];
        int n_ingredientes = partida.config.ingredientes_disponiveis;   /* [v2] */
        if (c >= '1' && c < '1' + n_ingredientes) {
            /* '1' vira PAO (0), '2' vira CARNE (1)... */
            jogo_empilhar(&partida, (Ingrediente)(c - '1'));
        } else if (c == 'd') {
            jogo_desfazer(&partida);
        } else if (c == 's') {
            jogo_servir(&partida);
        } else if (c == 't') {
            int n = atoi(linha + 1);                /* número depois do 't' */
            int i;
            if (n <= 0) n = 1;
            for (i = 0; i < n && !jogo_terminou(&partida); i++) {
                jogo_tick(&partida);
            }
        } else if (c == 'a') {
            imprimir_ajuda(&partida);
            continue;
        } else if (c == 'q') {
            break;
        } else {
            continue;                               /* linha vazia ou inválida */
        }
        imprimir_estado(&partida);
    }

    printf("\n=== FIM DE JOGO ===\n");
    printf("Pontuacao final: %d | Pedidos servidos: %d | Clientes perdidos: %d\n",
           partida.pontos, partida.servidos, partida.perdidos);

    jogo_destruir(&partida);
    return 0;
}
