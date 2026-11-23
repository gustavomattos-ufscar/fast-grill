/*
 * som.c — [v2] MÓDULO NOVO: geração e reprodução dos efeitos sonoros.
 *
 * =====================================================================
 *  COMO UM SOM É GERADO (o mínimo de física para entender o código)
 * =====================================================================
 * Som digital é uma lista de números (AMOSTRAS) que dizem a posição da
 * membrana do alto-falante a cada instante. Usamos 44100 amostras por
 * segundo (qualidade de CD), cada uma um 'short' de 16 bits
 * (-32767 a +32767).
 *
 * Uma NOTA é uma onda que se repete 'frequência' vezes por segundo:
 * 440 Hz = Lá; frequência maior = som mais agudo. A forma da onda muda o
 * "timbre":
 *     SENO       ~~~~   som suave (campainha)
 *     QUADRADA   _|‾|_  som de videogame antigo, mais "áspero"
 *     TRIÂNGULO  /\/\   meio-termo entre os dois
 *
 * Para cada amostra i calculamos:
 *     tempo = i / 44100                    (em segundos)
 *     fase  = parte fracionária de (frequência x tempo)   -> 0.0 a 1.0
 *     valor = onda(fase) x envelope x volume x 32767
 *
 * O ENVELOPE é um "controle de volume" que muda ao longo da nota: sobe
 * rápido no começo (5 ms) e desce até 0 no fim. Sem ele, cada nota
 * começaria e terminaria com um estalo ("click").
 *
 * Depois, a raylib transforma a lista de amostras em um Sound que pode
 * ser tocado com PlaySound (funções da raylib: InitAudioDevice,
 * LoadSoundFromWave, PlaySound, UnloadSound, CloseAudioDevice).
 */

#include <math.h>     /* sinf, fmodf, fabsf */
#include <stdlib.h>   /* malloc, free */
#include "raylib.h"
#include "som.h"

#define TAXA_AMOSTRAGEM 44100      /* amostras por segundo                 */
#define DOIS_PI 6.28318530718f
#define ATAQUE_SEGUNDOS 0.005f     /* tempo para o volume subir (5 ms)     */

/* Frequências (Hz) das notas usadas. Do4 = 261,63 Hz etc. */
#define SOL3 196.00f
#define DO4  261.63f
#define MI4  329.63f
#define FA4  349.23f
#define SOL4 392.00f
#define LA4  440.00f
#define DO5  523.25f
#define MI5  659.25f
#define SOL5 783.99f
#define LA5  880.00f
#define DO6  1046.50f
#define MI6  1318.51f

typedef enum { ONDA_SENO, ONDA_QUADRADA, ONDA_TRIANGULO } FormaOnda;

/* Estado do módulo (static = só este arquivo enxerga). */
static Sound sons[NUM_SONS];   /* um Sound para cada TipoSom           */
static bool carregados = false; /* os sons foram gerados com sucesso?  */
static bool ligado = true;      /* tecla M liga/desliga                */

/* Valor da onda (entre -1 e +1) em uma fase de 0.0 a 1.0. */
static float valor_onda(FormaOnda forma, float fase) {
    switch (forma) {
        case ONDA_SENO:
            return sinf(DOIS_PI * fase);
        case ONDA_QUADRADA:
            return (fase < 0.5f) ? 1.0f : -1.0f;      /* metade em cima, metade embaixo */
        case ONDA_TRIANGULO:
            return 4.0f * fabsf(fase - 0.5f) - 1.0f;  /* sobe e desce em linha reta */
    }
    return 0.0f;
}

/* Gera um som com 'num_notas' notas seguidas, cada uma com
 * 'duracao_nota' segundos. Ex.: {DO5, MI5, SOL5} toca Dó-Mi-Sol. */
static Sound criar_som(const float notas[], int num_notas, float duracao_nota,
                       FormaOnda forma, float volume) {
    int por_nota = (int)(duracao_nota * TAXA_AMOSTRAGEM);   /* amostras por nota */
    int total = por_nota * num_notas;
    int ataque = (int)(ATAQUE_SEGUNDOS * TAXA_AMOSTRAGEM);
    int n, i;
    Sound vazio = { 0 };

    /* Vetor de amostras alocado dinamicamente (Aula 09). */
    short *amostras = malloc(total * sizeof(short));
    if (amostras == NULL) {
        return vazio;
    }

    for (n = 0; n < num_notas; n++) {
        for (i = 0; i < por_nota; i++) {
            float tempo = (float)i / TAXA_AMOSTRAGEM;
            float fase = fmodf(notas[n] * tempo, 1.0f);

            /* Envelope: desce de 1 até 0 ao longo da nota... */
            float envelope = 1.0f - (float)i / por_nota;
            /* ...e nos primeiros 5 ms sobe de 0 até 1 (evita o estalo). */
            if (i < ataque) {
                envelope *= (float)i / ataque;
            }

            float valor = valor_onda(forma, fase) * envelope * volume;
            amostras[n * por_nota + i] = (short)(valor * 32767.0f);
        }
    }

    /* Wave = "som cru" da raylib: descrevemos o formato das amostras. */
    Wave onda;
    onda.frameCount = (unsigned int)total;   /* quantas amostras        */
    onda.sampleRate = TAXA_AMOSTRAGEM;       /* 44100 por segundo       */
    onda.sampleSize = 16;                    /* 16 bits = short         */
    onda.channels = 1;                       /* mono                    */
    onda.data = amostras;

    Sound som = LoadSoundFromWave(onda);     /* a raylib COPIA os dados */
    free(amostras);                          /* então podemos liberar   */
    return som;
}

void som_iniciar(void) {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        carregados = false;     /* sem placa de som: o jogo segue mudo */
        return;
    }

    /* Cada som: notas, duração de cada nota, forma da onda, volume (0 a 1).
     * A onda quadrada soa mais alta, por isso tem volume menor. */
    const float empilhar[] = { LA5 };
    const float desfazer[] = { LA4, MI4 };
    const float servir[]   = { DO5, MI5, SOL5, DO6 };
    const float erro[]     = { 150.0f, 110.0f };
    const float chegada[]  = { LA5, MI5 };
    const float vida[]     = { LA4, FA4, DO4 };
    const float nivel[]    = { DO5, MI5, SOL5, DO6, MI6 };
    const float fim[]      = { SOL4, MI4, DO4, SOL3 };
    const float menu[]     = { LA5 };
    const float iniciar[]  = { DO5, SOL5 };

    sons[SOM_EMPILHAR] = criar_som(empilhar, 1, 0.07f, ONDA_TRIANGULO, 0.45f);
    sons[SOM_DESFAZER] = criar_som(desfazer, 2, 0.05f, ONDA_TRIANGULO, 0.45f);
    sons[SOM_SERVIR]   = criar_som(servir,   4, 0.08f, ONDA_QUADRADA,  0.18f);
    sons[SOM_ERRO]     = criar_som(erro,     2, 0.12f, ONDA_QUADRADA,  0.20f);
    sons[SOM_CHEGADA]  = criar_som(chegada,  2, 0.18f, ONDA_SENO,      0.45f);
    sons[SOM_VIDA]     = criar_som(vida,     3, 0.14f, ONDA_TRIANGULO, 0.50f);
    sons[SOM_NIVEL]    = criar_som(nivel,    5, 0.07f, ONDA_SENO,      0.40f);
    sons[SOM_FIM]      = criar_som(fim,      4, 0.25f, ONDA_TRIANGULO, 0.50f);
    sons[SOM_MENU]     = criar_som(menu,     1, 0.04f, ONDA_SENO,      0.30f);
    sons[SOM_INICIAR]  = criar_som(iniciar,  2, 0.10f, ONDA_QUADRADA,  0.18f);
    carregados = true;
}

void som_encerrar(void) {
    int i;
    if (carregados) {
        for (i = 0; i < NUM_SONS; i++) {
            UnloadSound(sons[i]);
        }
        carregados = false;
    }
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
}

void som_tocar(TipoSom tipo) {
    if (!carregados || !ligado || (int)tipo < 0 || tipo >= NUM_SONS) {
        return;
    }
    PlaySound(sons[tipo]);
}

void som_alternar(void) {
    ligado = !ligado;
}

bool som_esta_ligado(void) {
    return ligado;
}
