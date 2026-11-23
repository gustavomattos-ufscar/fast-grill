# Registro de mudanças: Fast Grill

Este arquivo registra **toda** mudança feita no projeto, versão por versão: o quê, onde, por quê e como foi verificado.

> **Como achar as mudanças no código:** toda linha ou bloco alterado na versão 2 tem a etiqueta **`[v2]`** em um comentário. No VS Code, use `Ctrl+Shift+F` e procure `[v2]` para ver todos os pontos de uma vez.

---

## Versão 3 (14/11/2026): fonte nova

| O que | Como ficou |
|---|---|
| **Fonte** | Os textos trocaram a fonte pixelada padrão da raylib pela **Fredoka Bold** (Google Fonts, licença livre SIL OFL 1.1), em `assets/Fredoka-Bold.ttf` (licença em `assets/OFL-Fredoka.txt`). |
| **Código** | `ui.c`: todo `DrawText`/`MeasureText` virou `escrever`/`medir`, que usam `DrawTextEx`/`MeasureTextEx` com a fonte carregada. `ui.h`: novas `ui_carregar_fonte` e `ui_descarregar_fonte`, chamadas em `main.c` logo depois de `InitWindow` e antes de `CloseWindow`. Etiqueta **`[v3]`** nos comentários. |
| **Ajustes** | Nomes nos quadradinhos da receita: tamanho 10 → 13; "aguardando...": 12 → 14 (ficavam pequenos demais na fonte nova). |
| **Sem a fonte** | Se `assets/` não estiver ao lado do executável (nem na pasta de onde ele foi aberto), o jogo avisa no terminal e usa a fonte padrão, como antes. |
| **Verificação** | `make test`: 187 de 187. Capturas (`--captura`) das 5 telas conferidas: nenhum texto sai do lugar. |

---

## Versão 2 (10/11/2026): seletor de dificuldade + efeitos sonoros

### Backup da versão anterior

Antes de qualquer mudança foi feito um backup **completo** da pasta `Projeto` (código, relatórios, PDFs, executáveis):

```
Projeto/backups/backup_2026-11-10_v1_antes_dificuldade_e_som.zip   (56 arquivos, integridade verificada)
```

**Para voltar à versão 1:** extraia o `.zip` em uma pasta vazia; a pasta `fast-grill/` de dentro dele é o projeto exatamente como estava.

### Resumo

| O que | Como ficou |
|---|---|
| **Seletor de dificuldade** | O menu agora tem 3 cartões: **Fácil**, **Médio** e **Difícil**. Escolha com `1`/`2`/`3`, setas ou clique, e comece com `ENTER` ou com o botão. |
| **Fácil** | **Exatamente o jogo da versão 1.** Verificado: a mesma semente gera a mesma partida, tick por tick. |
| **Médio e Difícil** | Mais ingredientes (Bacon, Ovo, Cebola), receitas novas, menos paciência, clientes chegando mais rápido, receitas grandes desde o início, fila menor (Difícil) e **castigo por servir errado**. Em troca, cada ingrediente vale mais pontos. |
| **Efeitos sonoros** | 10 sons **gerados pelo próprio código** (sem arquivos de áudio): empilhar, desfazer, servir, erro, cliente chegando, vida perdida, subir de nível, fim de jogo, menu e início. Tecla **`M`** liga ou desliga. |
| **Testes** | Passaram de **143 para 187**, todos aprovados. Os testes antigos continuam valendo (rodando no Fácil). |

### As 3 dificuldades

Os números ficam numa tabela só, `CONFIGS[]`, no início do `src/jogo.c`. O menu lê essa tabela, então mudar um número lá muda o jogo **e** o texto do menu.

| Regra | Fácil (= v1) | Médio | Difícil |
|---|---|---|---|
| Ingredientes disponíveis | 5 | 7 (+Bacon, Ovo) | 8 (+Cebola) |
| Paciência do cliente | 8 + 3 por item | 6 + 2 por item | 5 + 2 por item |
| Exemplo: receita de 4 itens | 20 ticks | 14 ticks | 13 ticks |
| Novo cliente a cada (início → mínimo) | 10 → 4 ticks | 8 → 3 ticks | 6 → 3 ticks |
| Maior receita no nível 1 | 4 itens | 5 itens | 6 itens |
| Servidos para subir de nível | 5 | 4 | 3 |
| Fila lota com | 5 | 5 | **4** |
| Servir errado | sem castigo | **−2 de paciência** | **−3 de paciência** |
| Pontos por ingrediente | 10 | 15 | 20 |

**Por que essas mudanças deixam mais difícil:**

- **Mais ingredientes:** mais botões para procurar e receitas mais variadas. É mais fácil errar.
- **Menos paciência e chegadas mais rápidas:** a fila enche mais depressa e sobra menos tempo por pedido.
- **Fila de 4 no Difícil:** a "fila lotada" (perda de vida) acontece mais cedo.
- **Castigo por servir errado:** não dá mais para "chutar" o servir até acertar. Cada tentativa errada aproxima a desistência do cliente. A penalidade nunca zera a paciência sozinha (para em 1), porque a desistência continua acontecendo só no `jogo_tick`. Assim existe um único lugar no código que cuida disso.

### Receitas novas

Elas ficam **no fim** do cardápio, de propósito: assim a lista de receitas sorteáveis do Fácil não muda e a mesma semente gera a mesma partida da v1.

| Receita | Itens (base → topo) | Aparece em |
|---|---|---|
| Pao com Ovo | Pão, Ovo, Pão | Médio, Difícil |
| X-Bacon | Pão, Carne, Bacon, Pão | Médio, Difícil |
| X-Egg | Pão, Carne, Ovo, Queijo, Pão | Médio, Difícil |
| Bacon Salada | Pão, Bacon, Alface, Tomate, Pão | Médio, Difícil |
| X-Bacon Egg | Pão, Carne, Bacon, Ovo, Queijo, Pão | Médio, Difícil |
| Completo | Pão, Carne, Bacon, Ovo, Queijo, Alface, Pão | Médio, Difícil |
| Cebolada | Pão, Carne, Cebola, Pão | Difícil |
| Smash Cebola | Pão, Carne, Cebola, Carne, Queijo, Pão | Difícil |
| Supremo | Pão, Carne, Cebola, Bacon, Ovo, Tomate, Pão | Difícil |

A regra que decide se uma receita pode aparecer é `receita_disponivel()`, em `pedido.c`. Ela vale se **todos** os ingredientes da receita estão entre os N primeiros do enum, com N = 5, 7 ou 8. Por isso os ingredientes novos foram colocados **no fim** do `enum Ingrediente`.

### Efeitos sonoros

| Quando | Som | Notas | Onda |
|---|---|---|---|
| Ingrediente no prato | "pop" | Lá5 | triângulo |
| Desfazer | "pop" descendo | Lá4 → Mi4 | triângulo |
| Pedido servido | arpejo subindo | Dó5 Mi5 Sol5 Dó6 | quadrada |
| Ação recusada / prato errado | zumbido grave | 150 Hz → 110 Hz | quadrada |
| Cliente chegou | campainha "plim-plom" | Lá5 → Mi5 | seno |
| Perdeu uma vida | três notas descendo | Lá4 Fá4 Dó4 | triângulo |
| Subiu de nível | arpejo rápido e agudo | Dó5 … Mi6 | seno |
| Fim de jogo | notas longas descendo | Sol4 Mi4 Dó4 Sol3 | triângulo |
| Menu / tecla M | clique | Lá5 | seno |
| Partida começou | duas notas | Dó5 Sol5 | quadrada |

**Como funciona, em 3 passos:**

1. **O núcleo anota.** O `jogo.c` não sabe o que é som. Quando algo acontece, ele só marca um campo da struct `Eventos` da partida (ex.: `p->eventos.serviu = true`).
2. **O `main.c` toca.** A cada quadro, `tocar_eventos()` olha essas anotações, chama `som_tocar(...)` e depois `jogo_limpar_eventos()`.
3. **O `som.c` gera os sons.** Ao abrir o jogo, ele calcula as ondas (seno, quadrada, triângulo) amostra por amostra, com um "envelope" que evita estalos, e entrega para a raylib (`LoadSoundFromWave`). A explicação completa está no comentário do topo de `src/som.c`.

Vantagens de gerar os sons por código:

- não há arquivo de áudio para perder;
- não há direitos autorais de terceiros;
- tudo continua sendo código do grupo, exceto a raylib.

Se o computador não tiver saída de áudio, o jogo funciona normalmente, só que mudo.

### Mudanças arquivo por arquivo

| Arquivo | O que mudou |
|---|---|
| `src/pedido.h` | + `BACON`, `OVO`, `CEBOLA` no **fim** do enum; `NUM_INGREDIENTES` 5 → 8; + `NUM_INGREDIENTES_BASICOS`; + struct **`RegrasPedido`**; + `receita_disponivel()`; `pedido_criar` e `pedido_gerar` recebem `const RegrasPedido *`. **Removidos:** `PONTOS_POR_INGREDIENTE`, `PACIENCIA_BASE`, `PACIENCIA_POR_INGREDIENTE` (agora vêm da dificuldade). |
| `src/pedido.c` | Nomes dos 3 ingredientes; 9 receitas novas no fim do cardápio; `receita_disponivel()`; o sorteio filtra por tamanho **e** por ingredientes disponíveis; paciência e valor calculados com as regras recebidas. |
| `src/jogo.h` | + `enum Dificuldade`; + struct **`ConfigDificuldade`**; + struct **`Eventos`**; `Partida` ganhou `dificuldade`, `config` e `eventos`; `jogo_iniciar` ganhou o parâmetro `dificuldade`; + `jogo_config()`; + `jogo_limpar_eventos()`. **Removidos:** `MAX_PEDIDOS_FILA`, `INTERVALO_INICIAL`, `INTERVALO_MINIMO`, `PEDIDOS_POR_NIVEL`, `TAMANHO_RECEITA_INICIAL` (agora estão na tabela `CONFIGS`). |
| `src/jogo.c` | + tabela **`CONFIGS[]`**; capacidade da fila, intervalos, nível e tamanho das receitas vêm da config; + `aplicar_penalidade()` ao servir errado; empilhar ingrediente fora da dificuldade é recusado; cada acontecimento marca um `Evento`. **O contador de acertos, a fila e a pilha não mudaram.** |
| `src/som.h`, `src/som.c` | **Arquivos novos**: geração e reprodução dos 10 efeitos sonoros. |
| `src/ui.h` | + ações `ACAO_ESCOLHER_DIFICULDADE`, `ACAO_VOLTAR_MENU`, `ACAO_ALTERNAR_SOM`; `Acao` ganhou o campo `dificuldade`; `ui_ler_acao` e `ui_desenhar` recebem mais parâmetros. |
| `src/ui.c` | Menu com cartões de dificuldade e botão "Comecar"; etiqueta da dificuldade no cabeçalho; 5, 7 ou 8 botões de ingrediente (tamanho calculado); cores de Bacon, Ovo e Cebola; fila com 5 ou 4 espaços; indicador de som; tela de fim mostra a dificuldade e a opção de voltar ao menu. |
| `src/main.c` | Guarda a dificuldade escolhida; liga e desliga o áudio; `tocar_eventos()`; tecla `M`; na tela de fim, `Backspace` volta ao menu; `--captura` também gera `captura_dificil.png`. |
| `src/main_texto.c` | 2º argumento de dificuldade (`fast-grill-texto 42 dificil`); ajuda lista os ingredientes da dificuldade; aceita teclas `1` a `8`. |
| `tests/teste_fila.c` | Só a criação do pedido de teste (nova assinatura de `pedido_criar`). |
| `tests/teste_jogo.c` | Chamadas atualizadas (testes antigos rodam no Fácil) + **44 testes novos** em 4 seções `[v2]`: ingredientes por dificuldade, tabela de dificuldades, penalidade e eventos. Também testa a fila lotada com 4 no Difícil. |
| `Makefile` | `src/som.c` entrou na compilação do jogo com janela. |
| `docs/` | Capturas novas (`captura_menu.png` com o seletor, `captura_dificil.png`); `demo_texto_dificil.txt` + `demo_comandos_dificil.txt`; `saida_testes.txt` atualizado. |
| `README.md`, `entenda.md` | Atualizados com dificuldade, som e novos controles. |
| `Entrega/Fase2/02_relatorio_fase_2.tex` | Atualizado para refletir a versão 2 (o relatório precisa bater com o repositório). |

### Controles novos

| Onde | Tecla | Ação |
|---|---|---|
| Menu | `1` `2` `3` / `←` `→` / clique | escolhe a dificuldade |
| Menu | `ENTER` / botão "Comecar" | começa |
| Partida | `1` a `8` | ingredientes (quantos aparecem depende da dificuldade) |
| Fim | `ENTER` | joga de novo na mesma dificuldade |
| Fim | `Backspace` | volta ao menu (trocar dificuldade) |
| Qualquer tela | `M` | liga/desliga o som |

### Como foi verificado

1. **Compilação** sem nenhum aviso (`-Wall -Wextra`) do jogo, do modo texto e dos testes.
2. **`make test`: 187 de 187 testes aprovados** (fila 37, pilha 31, jogo 119).
3. **O Fácil é idêntico à v1.** A partida de demonstração (`docs/demo_comandos.txt`, semente 42) foi rodada na v1 (do backup) e na v2. A parte do jogo da saída é **idêntica linha por linha**; só mudou o cabeçalho, que agora mostra a dificuldade.
4. **Os testes novos pegam erros.** Dois bugs propositais foram colocados numa cópia do `jogo.c`:
   - tirar o "piso" da penalidade;
   - mudar o intervalo do Fácil de 10 para 9.

   Os testes "a penalidade para em 1" e "Facil ... (igual a v1)" falharam, como esperado.
5. **Sons.** Um programa de verificação ligou o áudio e confirmou que os 10 sons foram gerados e carregados (0,04 s a 1 s cada), e que o liga/desliga funciona. O jogo rodou normalmente com o áudio ligado. *Ouvir* os sons depende de vocês: abram o jogo com o volume ligado.
6. **Interface conferida por imagens** (`make capturas`): menu com seletor, partida no Fácil, prato errado, fim de jogo e Difícil com 8 ingredientes, fila de 4 e mensagem de penalidade.

---

## Versão 1 (10/11/2026): Fase 2

Primeira versão jogável: TADs Fila (vetor circular) e Pilha (vetor), núcleo com contador de acertos, interface raylib, modo texto e 143 testes. Detalhes no `entenda.md` e no relatório da Fase 2.
