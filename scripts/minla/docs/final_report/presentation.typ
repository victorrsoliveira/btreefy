// ─────────────────────────────────────────────────────────────────────────────
// presentation.typ — MinLA em Behavior Trees: uma solução com Simulated Annealing
// Disciplina: Otimização & Pesquisa Operacional · UFAL PPGI · 2026
// Autor: Victor R. S. de Oliveira
// Tecnologia: Typst + touying (university theme) + cetz
// ─────────────────────────────────────────────────────────────────────────────

#import "@preview/touying:0.5.2": *
#import themes.university: *
#import "@preview/cetz:0.3.4": canvas, draw

// ── Tema e configuração global ────────────────────────────────────────────────
#show: university-theme.with(
  aspect-ratio: "16-9",
  config-page(margin: (top: 3em, bottom: 2em, left: 3em, right: 2em)),
  config-info(
    title: [O problema de _Minimum Linear Arrangement_ em Behavior Trees],
    subtitle: [Uma solução com Simulated Annealing],
    author: [Victor R. S. de Oliveira],
    date: datetime.today().display("[day]/[month]/[year]"),
    institution: [Otimização & Pesquisa Operacional \
    PPGI —  Universidade Federal de Alagoas (UFAL)],
  ),
)

#set text(font: "Liberation Sans", size: 18pt, lang: "pt")
#set math.equation(numbering: none)
#show math.equation: set text(size: 17pt)

// ── Helpers ───────────────────────────────────────────────────────────────────
#let hl(body) = text(fill: rgb("#1a5fa8"), weight: "bold", body)
#let note(body) = text(fill: rgb("#555555"), size: 14pt, body)
#let tag(body) = box(
  fill: rgb("#e8f0fb"), stroke: 0.5pt + rgb("#1a5fa8"),
  inset: (x: 6pt, y: 3pt), radius: 3pt,
  text(fill: rgb("#1a5fa8"), size: 13pt, body)
)

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 1 — Capa (gerada automaticamente pelo tema university)
// ─────────────────────────────────────────────────────────────────────────────
#title-slide()

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 2 — Motivação: BTreeFy e o problema de layout
// ─────────────────────────────────────────────────────────────────────────────
= Motivação

== O contexto: BTreeFy

#grid(columns: (1fr, 1fr), gutter: 24pt)[
  *Behavior Trees (BTs)* são usadas extensivamente em robótica e sistemas embarcados para controle de comportamento.

  #v(10pt)
  *BTreeFy* armazena os nós como um *array flat 1D* de `btf_node` em C:

  ```c
  struct btf_node {
    uint32_t parent;
    uint32_t child;
    uint32_t sibling;
    // ...
  };
  ```
][
  #v(12pt)
  *O problema:* a travessia DFS segue ponteiros inteiros (índices no array).

  #v(10pt)
  Nós conectados que estão *longe no array* causam:
  - Evicção de linha de cache
  - Recarga desnecessária (_cache miss_)
  - Degradação de desempenho em MCUs

  #v(10pt)
  #hl[Como ordenar os nós no array para minimizar esses saltos?]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 3 — Modelo do Problema: Grafo LCRS
// ─────────────────────────────────────────────────────────────────────────────
= Modelo do Problema

== Representação como grafo

A BT é modelada como um grafo não-dirigido ponderado $G = (V, E, W)$:

#v(8pt)
#grid(columns: (1fr, 1fr), gutter: 28pt)[
  *Vértices:*
  - $V = {0, 1, ..., n-1}$ — um vértice por nó

  #v(6pt)
  *Arestas* (campos LCRS não-nulos):
  - $(i, "node.parent")$
  - $(i, "node.child")$
  - $(i, "node.sibling")$
  - Duplicatas removidas → $|E| = O(n)$

  #v(6pt)
  *Pesos:* $W_(i j) = 1$ (uniforme — linha de base)
][
  #v(8pt)
  #tag[Estrutura LCRS]
  #v(8pt)
  _Left-Child Right-Sibling:_ cada nó aponta para seu filho mais à esquerda e para o próximo irmão. Minimiza alocação de memória em sistemas embarcados.

  #v(12pt)
  #note[As arestas LCRS criam um grafo esparso (não-simples) — o grafo não é uma árvore pura devido às arestas de irmão.]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 4 — Toy Example: BT de 5 nós
// ─────────────────────────────────────────────────────────────────────────────
= Modelo do Problema

== Exemplo ilustrativo — 5 nós

#grid(columns: (1.1fr, 1fr), gutter: 20pt, align: top)[

  // ── Diagrama 1: layout original DFS pré-ordem ──────────────────────────────
  #align(center)[
    #text(size: 13pt, style: "italic", fill: rgb("#444444"))[Árvore original (DFS pré-ordem) — custo: 12]
    #v(4pt)
    #canvas(length: 0.75cm, {
      import draw: *

      let c_ctrl = rgb("#dce8f7")
      let s_ctrl = 1pt + rgb("#1a5fa8")
      let c_act  = rgb("#e8f7e8")
      let s_act  = 1pt + rgb("#2d8a2d")
      let s_edge = (stroke: 0.8pt + rgb("#444444"))
      let s_sib  = (stroke: (paint: rgb("#e06000"), dash: "dashed", thickness: 0.7pt))

      let pos = (
        (0,    0),     // 0  Fallback
        (-1.5, -1.6),  // 1  Sequence
        (-2.5, -3.1),  // 2  Action A
        (-0.5, -3.1),  // 3  Action B
        ( 1.2, -1.6),  // 4  Action C
      )

      // Arestas (desenhadas antes dos nós)
      for (a, b) in ((0,1),(0,4),(1,2),(1,3)) {
        let (ax, ay) = pos.at(a)
        let (bx, by) = pos.at(b)
        line((ax, ay), (bx, by), ..s_edge)
      }
      for (a, b) in ((1,4),(2,3)) {
        let (ax, ay) = pos.at(a)
        let (bx, by) = pos.at(b)
        line((ax, ay), (bx, by), ..s_sib)
      }

      // Nó 0: Fallback — dois content com offset vertical (sem linebreak)
      let (fx, fy) = pos.at(0)
      rect((fx - 0.95, fy - 0.44), (fx + 0.95, fy + 0.44), fill: c_ctrl, stroke: s_ctrl, radius: 0.08)
      content((fx, fy + 0.22), align(center)[#text(size: 7.5pt, weight: "bold")[Fallback]])
      content((fx, fy - 0.22), align(center)[#text(size: 7pt)[[0]]])

      // Nó 1: Sequence — dois content com offset vertical
      let (sx, sy) = pos.at(1)
      rect((sx - 0.95, sy - 0.44), (sx + 0.95, sy + 0.44), fill: c_ctrl, stroke: s_ctrl, radius: 0.08)
      content((sx, sy + 0.22), align(center)[#text(size: 7.5pt, weight: "bold")[Sequence]])
      content((sx, sy - 0.22), align(center)[#text(size: 7pt)[[1]]])

      // Nó 2: Action A
      let (ax2, ay2) = pos.at(2)
      circle((ax2, ay2), radius: 0.53, fill: c_act, stroke: s_act)
      content((ax2, ay2), align(center)[
        #text(size: 8pt, weight: "bold")[A [2]]
      ])

      // Nó 3: Action B
      let (bx3, by3) = pos.at(3)
      circle((bx3, by3), radius: 0.53, fill: c_act, stroke: s_act)
      content((bx3, by3), align(center)[
        #text(size: 8pt, weight: "bold")[B [3]]
      ])

      // Nó 4: Action C
      let (cx4, cy4) = pos.at(4)
      circle((cx4, cy4), radius: 0.53, fill: c_act, stroke: s_act)
      content((cx4, cy4), align(center)[
        #text(size: 8pt, weight: "bold")[C [4]]
      ])
    })
  ]

  #v(10pt)

  // ── Diagrama 2: layout ótimo MILP — y* = [1, 2, 4, 3, 0] ──────────────────
  // Novos slots: Fallback→1, Sequence→2, A→4, B→3, C→0
  #align(center)[
    #text(size: 13pt, style: "italic", fill: rgb("#1a5fa8"))[Layout ótimo (MILP) — custo: 8]
    #v(4pt)
    #canvas(length: 0.75cm, {
      import draw: *

      let c_ctrl = rgb("#dce8f7")
      let s_ctrl = 1pt + rgb("#1a5fa8")
      let c_act  = rgb("#e8f7e8")
      let s_act  = 1pt + rgb("#2d8a2d")
      let s_edge = (stroke: 0.8pt + rgb("#444444"))
      let s_sib  = (stroke: (paint: rgb("#e06000"), dash: "dashed", thickness: 0.7pt))

      // Mesma estrutura de árvore — apenas os slots mudam nos rótulos
      let pos = (
        (0,    0),     // Fallback → slot 1
        (-1.5, -1.6),  // Sequence → slot 2
        (-2.5, -3.1),  // Action A → slot 4
        (-0.5, -3.1),  // Action B → slot 3
        ( 1.2, -1.6),  // Action C → slot 0
      )

      // Arestas (mesma topologia)
      for (a, b) in ((0,1),(0,4),(1,2),(1,3)) {
        let (ax, ay) = pos.at(a)
        let (bx, by) = pos.at(b)
        line((ax, ay), (bx, by), ..s_edge)
      }
      for (a, b) in ((1,4),(2,3)) {
        let (ax, ay) = pos.at(a)
        let (bx, by) = pos.at(b)
        line((ax, ay), (bx, by), ..s_sib)
      }

      // Nó 0: Fallback → slot 1
      let (fx, fy) = pos.at(0)
      rect((fx - 0.95, fy - 0.44), (fx + 0.95, fy + 0.44), fill: c_ctrl, stroke: s_ctrl, radius: 0.08)
      content((fx, fy + 0.22), align(center)[#text(size: 7.5pt, weight: "bold")[Fallback]])
      content((fx, fy - 0.22), align(center)[#text(size: 7pt)[[1]]])

      // Nó 1: Sequence → slot 2
      let (sx, sy) = pos.at(1)
      rect((sx - 0.95, sy - 0.44), (sx + 0.95, sy + 0.44), fill: c_ctrl, stroke: s_ctrl, radius: 0.08)
      content((sx, sy + 0.22), align(center)[#text(size: 7.5pt, weight: "bold")[Sequence]])
      content((sx, sy - 0.22), align(center)[#text(size: 7pt)[[2]]])

      // Nó 2: Action A → slot 4
      let (ax2, ay2) = pos.at(2)
      circle((ax2, ay2), radius: 0.53, fill: c_act, stroke: s_act)
      content((ax2, ay2), align(center)[
        #text(size: 8pt, weight: "bold")[A [4]]
      ])

      // Nó 3: Action B → slot 3
      let (bx3, by3) = pos.at(3)
      circle((bx3, by3), radius: 0.53, fill: c_act, stroke: s_act)
      content((bx3, by3), align(center)[
        #text(size: 8pt, weight: "bold")[B [3]]
      ])

      // Nó 4: Action C → slot 0
      let (cx4, cy4) = pos.at(4)
      circle((cx4, cy4), radius: 0.53, fill: c_act, stroke: s_act)
      content((cx4, cy4), align(center)[
        #text(size: 8pt, weight: "bold")[C [0]]
      ])
    })
  ]

][
  #v(6pt)
  *Arestas LCRS:* $E = {(0,1),(0,4),(1,2),(1,3),(1,4),(2,3)}$ — 6 arestas

  #v(12pt)
  #table(
    columns: (auto, auto, auto),
    stroke: 0.5pt + rgb("#cccccc"),
    fill: (_, row) => if row == 0 { rgb("#f0f4fa") } else { white },
    inset: 7pt,
    [*Layout*], [*Ordem no array*], [*Custo*],
    [DFS pré-ordem], [`[F, S, A, B, C]`], hl[12],
    [Ótimo (MILP)],  [`[C, F, S, B, A]`], hl[8],
  )

  #v(10pt)
  #note[Legenda: — parent/child  ╌ sibling]
  #v(4pt)
  #hl[Redução de 33%] apenas reordenando o array.
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 5 — Formulação MinLA
// ─────────────────────────────────────────────────────────────────────────────
= Formulação Matemática

== Minimum Linear Arrangement (MinLA)

Encontrar uma bijeção $y : V -> {0, 1, ..., n-1}$ que minimize:

#v(8pt)
$ min_y quad sum_((i,j) in E) W_(i j) dot |y_i - y_j| $
#v(4pt)
$ "sujeito a:" quad y_i != y_j quad forall i != j $

#v(16pt)
#grid(columns: (1fr, 1fr), gutter: 24pt)[
  *Interpretação:*
  - $y_i$ = slot (posição) do nó $i$ no array
  - $|y_i - y_j|$ = distância entre nós adjacentes na memória
  - Minimizar a soma ponderada dessas distâncias
][
  #tag[Complexidade]
  #v(8pt)
  MinLA é *NP-Difícil* para grafos gerais.
  #note[(Garey & Johnson, 1979)]

  #v(8pt)
  Grafos BT são árvores esparsas, mas a formulação LCRS (com arestas de irmão) e a extensão para pesos re-introduzem a dificuldade computacional.
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 6 — Abordagem Exata: MILP (resumido)
// ─────────────────────────────────────────────────────────────────────────────
= Abordagem Exata

== Mixed-Integer Linear Program (MILP)

#grid(columns: (1fr, 1fr), gutter: 24pt)[
  *Linearização do valor absoluto:*

  Variável auxiliar $d_(i j) >= 0$ por aresta:
  $ d_(i j) >= y_i - y_j $
  $ d_(i j) >= y_j - y_i $
  Objetivo: $min sum W_(i j) dot d_(i j)$

  #v(8pt)
  *Slots únicos (Big-M):*

  Variável binária $z_(i j) in {0,1}$ por par $i != j$:
  $ y_i - y_j >= 1 - n z_(i j) $
  $ y_j - y_i >= 1 - n(1 - z_(i j)) $
][
  #tag[Contagem de variáveis]
  #v(8pt)
  #table(
    columns: (auto, auto, auto),
    stroke: 0.5pt + rgb("#cccccc"),
    fill: (_, row) => if row == 0 { rgb("#f0f4fa") } else { white },
    inset: 6pt,
    [*n*], [*Vars. binárias* $z_(i j)$], [*Tempo*],
    [5],  [20],   [< 1 s],
    [10], [90],   [~30 s],
    [12], [132],  [~173 s],
    [15], [210],  [*timeout*],
    [20], [380],  [*timeout*],
  )
  #v(6pt)
  #hl[O crescimento $O(n^2)$ torna o MILP intratável para $n >= 15$.]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 7 — O Muro Exponencial
// ─────────────────────────────────────────────────────────────────────────────
= O Muro Exponencial

== Análise empírica de escalabilidade

#grid(columns: (1.5fr, 1fr), gutter: 20pt, align: horizon)[
  #image("../figures/scaling_wall.png", width: 100%)
][
  *Protocolo:*
  - $n in {5, 8, 10, 12, 15, 20}$
  - 5 instâncias aleatórias por $n$
  - Limite: 300 s (5 min) por instância
  - Solver: PuLP / CBC

  #v(10pt)
  *Observações:*
  - $n <= 10$ → sempre ótimo em < 60 s
  - $n = 12$ → borda do timeout (83–300 s)
  - $n >= 15$ → #hl[todas as instâncias estouram]

  #v(10pt)
  #note[O muro empírico está em $n approx 12$–$15$. Instâncias reais de BT típicas têm $n = 20$–$200$.]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 8 — Meta-heurística: SA — Visão Geral
// ─────────────────────────────────────────────────────────────────────────────
= Meta-heurística

== Simulated Annealing — três componentes

#v(10pt)
#grid(columns: (1fr, 1fr, 1fr), gutter: 16pt)[
  #rect(fill: rgb("#f0f4fa"), stroke: 0.5pt + rgb("#1a5fa8"), inset: 14pt, radius: 4pt)[
    #hl[1. Espaço de estados]
    #v(8pt)
    Conjunto de todas as *permutações* de $n$ nós.
    #v(6pt)
    $|S| = n!$ — intratável por busca exaustiva.
  ]
][
  #rect(fill: rgb("#f0f4fa"), stroke: 0.5pt + rgb("#1a5fa8"), inset: 14pt, radius: 4pt)[
    #hl[2. Vizinhança]
    #v(8pt)
    *Swap* de dois nós aleatórios $a, b$.
    #v(6pt)
    Atualização incremental de custo: $O(deg(a) + deg(b))$ — não $O(|E|)$.
  ]
][
  #rect(fill: rgb("#f0f4fa"), stroke: 0.5pt + rgb("#1a5fa8"), inset: 14pt, radius: 4pt)[
    #hl[3. Aceitação]
    #v(8pt)
    $Delta <= 0$: aceitar sempre.
    #v(4pt)
    $Delta > 0$: aceitar com probabilidade
    $ P = e^(-Delta / T) $
    (critério de Metropolis)
  ]
]

#v(14pt)
#note[Resfriamento geométrico: $T_(k+1) = alpha dot T_k$, parada em $T < 10^(-4)$. Temperatura inicial $T_0$ calibrada para 80% de aceitação de pioras.]

// ── Pseudocódigo reutilizado nos 3 slides seguintes ───────────────────────────
#let pseudo = rect(fill: rgb("#f8f8f8"), stroke: 0.5pt + rgb("#888888"), inset: 14pt, radius: 3pt)[
  #set text(size: 13.5pt, font: "Liberation Mono")
  *Input:* grafo $G$, $alpha$, $T_"min"$\
  $y arrow.l$ #text(fill: rgb("#1a5fa8"), weight: "bold")[Fiedler\_permutation]$(G)$\
  $C arrow.l$ MinLA\_cost$(G, y)$\
  $C_"best" arrow.l C$; $quad y_"best" arrow.l y$\
  $T arrow.l$ #text(fill: rgb("#888888"))[initial\_temperature]$(G, y)$\
  *while* $T > T_"min"$:\
  #h(14pt)$a, b arrow.l$ #text(fill: rgb("#888888"))[produce\_neighbors\_swap]$(G.n)$\
  #h(14pt)$Delta arrow.l$ #text(fill: rgb("#888888"))[incremental\_cost]$(G, y, a, b)$\
  #h(14pt)*if* $Delta <= 0$ *or* rand$() < e^(-Delta\/T)$:\
  #h(28pt)swap $y_a, y_b$; $C += Delta$\
  #h(28pt)*if* $C < C_"best"$:\
  #h(42pt)$C_"best" arrow.l C$; $quad y_"best" arrow.l y$\
  #h(14pt)$T arrow.l alpha dot T$\
  *return* $y_"best"$
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 9 — Meta-heurística: Inicialização Espectral (Fiedler)
// ─────────────────────────────────────────────────────────────────────────────
= Meta-heurística

== Algoritmo SA — inicialização espectral

#v(6pt)
#grid(columns: (1.1fr, 0.9fr), gutter: 24pt, align: top)[
  #pseudo
][
  #v(4pt)
  *Vetor de Fiedler*

  #v(6pt)
  1. Montar o Laplaciano ponderado $L = D - A$
  2. Computar os autovetores de $L$ (scipy `eigh`)
  3. *Vetor de Fiedler* = autovetor do 2º menor autovalor $lambda_2 > 0$
  4. Ordenar nós pelo componente → permutação inicial $y^0$

  #v(10pt)
  *Pettis & Hansen (1990)*: ordenação minimiza um _proxy_ de transições inter-página — o mesmo que MinLA.
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 9 — Meta-heurística: Inicialização Espectral (Fiedler)
// ─────────────────────────────────────────────────────────────────────────────

== Algoritmo SA — inicialização espectral

#v(6pt)
#grid(columns: (1.1fr, 0.9fr), gutter: 24pt, align: top)[
  #pseudo
][
  No exemplo inicial, permutação inicial com vetor de Fiedler já retorna permutação ótima.

  #v(10pt)
  #table(
    columns: (auto, auto),
    stroke: 0.5pt + rgb("#cccccc"),
    fill: (_, row) => if row == 0 { rgb("#f0f4fa") } else { white },
    inset: 6pt,
    [*Init*], [*Custo (toy, n=5)*],
    [Aleatória], [12.0],
    [Fiedler],   hl[8.0 — ótimo],
  )
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 10 — Meta-heurística: Temperatura inicial
// ─────────────────────────────────────────────────────────────────────────────

== Algoritmo SA — temperatura inicial

#v(6pt)
#grid(columns: (1.1fr, 0.9fr), gutter: 24pt, align: top)[
  #let pseudo2 = rect(fill: rgb("#f8f8f8"), stroke: 0.5pt + rgb("#888888"), inset: 14pt, radius: 3pt)[
    #set text(size: 13.5pt, font: "Liberation Mono")
    *Input:* grafo $G$, $alpha$, $T_"min"$\
    $y arrow.l$ #text(fill: rgb("#888888"))[Fiedler\_permutation]$(G)$\
    $C arrow.l$ MinLA\_cost$(G, y)$\
    $C_"best" arrow.l C$; $quad y_"best" arrow.l y$\
    $T arrow.l$ #text(fill: rgb("#1a5fa8"), weight: "bold")[initial\_temperature]$(G, y)$\
    *while* $T > T_"min"$:\
    #h(14pt)$a, b arrow.l$ #text(fill: rgb("#888888"))[produce\_neighbors\_swap]$(G.n)$\
    #h(14pt)$Delta arrow.l$ #text(fill: rgb("#888888"))[incremental\_cost]$(G, y, a, b)$\
    #h(14pt)*if* $Delta <= 0$ *or* rand$() < e^(-Delta\/T)$:\
    #h(28pt)swap $y_a, y_b$; $C += Delta$\
    #h(28pt)*if* $C < C_"best"$:\
    #h(42pt)$C_"best" arrow.l C$; $quad y_"best" arrow.l y$\
    #h(14pt)$T arrow.l alpha dot T$\
    *return* $y_"best"$
  ]
  #pseudo2
][
  #v(4pt)
  *Temperatura inicial — `initial_temperature`*

  #v(8pt)
  Realiza uma _caminhada aleatória_ de 100 swaps a partir de $y^0$, amostrando apenas os movimentos que *pioram* o custo ($Delta > 0$). Com a média desses deltas $overline(Delta)^+$, aplica a fórmula de Kirkpatrick:

  $ T_0 = -frac(overline(Delta)^+, ln(p_0)), quad p_0 = 0.8 $

  Garante *80%* de aceitação de pioras típicas no início — suficiente para escapar de mínimos locais sem tornar a busca puramente aleatória.
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 11 — Meta-heurística: Geração de vizinhança
// ─────────────────────────────────────────────────────────────────────────────

== Algoritmo SA — geração de vizinhança

#v(6pt)
#grid(columns: (1.1fr, 0.9fr), gutter: 24pt, align: top)[
  #let pseudo_neigh = rect(fill: rgb("#f8f8f8"), stroke: 0.5pt + rgb("#888888"), inset: 14pt, radius: 3pt)[
    #set text(size: 13.5pt, font: "Liberation Mono")
    *Input:* grafo $G$, $alpha$, $T_"min"$\
    $y arrow.l$ #text(fill: rgb("#888888"))[Fiedler\_permutation]$(G)$\
    $C arrow.l$ MinLA\_cost$(G, y)$\
    $C_"best" arrow.l C$; $quad y_"best" arrow.l y$\
    $T arrow.l$ #text(fill: rgb("#888888"))[initial\_temperature]$(G, y)$\
    *while* $T > T_"min"$:\
    #h(14pt)$a, b arrow.l$ #text(fill: rgb("#1a5fa8"), weight: "bold")[produce\_neighbors\_swap]$(G.n)$\
    #h(14pt)$Delta arrow.l$ #text(fill: rgb("#888888"))[incremental\_cost]$(G, y, a, b)$\
    #h(14pt)*if* $Delta <= 0$ *or* rand$() < e^(-Delta\/T)$:\
    #h(28pt)swap $y_a, y_b$; $C += Delta$\
    #h(28pt)*if* $C < C_"best"$:\
    #h(42pt)$C_"best" arrow.l C$; $quad y_"best" arrow.l y$\
    #h(14pt)$T arrow.l alpha dot T$\
    *return* $y_"best"$
  ]
  #pseudo_neigh
][
  #v(4pt)
  *Geração de vizinhança — `produce_neighbors_swap`*

  #v(8pt)
  Para explorar o espaço de busca, a cada iteração, sorteia-se aleatoriamente dois nós distintos da árvore para realizar uma troca (swap) de posições no array $y$.

  O sorteio garante $a != b$ rejeitando tentativas onde os nós são iguais, mantendo a simplicidade da amostragem sem viés.
]


// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 11 — Meta-heurística: Custo incremental e aceitação
// ─────────────────────────────────────────────────────────────────────────────

== Algoritmo SA — vizinhança e aceitação

#v(6pt)
#grid(columns: (1.1fr, 0.9fr), gutter: 24pt, align: top)[
  #let pseudo3 = rect(fill: rgb("#f8f8f8"), stroke: 0.5pt + rgb("#888888"), inset: 14pt, radius: 3pt)[
    #set text(size: 13.5pt, font: "Liberation Mono")
    *Input:* grafo $G$, $alpha$, $T_"min"$\
    $y arrow.l$ #text(fill: rgb("#888888"))[Fiedler\_permutation]$(G)$\
    $C arrow.l$ MinLA\_cost$(G, y)$\
    $C_"best" arrow.l C$; $quad y_"best" arrow.l y$\
    $T arrow.l$ #text(fill: rgb("#888888"))[initial\_temperature]$(G, y)$\
    *while* $T > T_"min"$:\
    #h(14pt)$a, b arrow.l$ #text(fill: rgb("#888888"))[produce\_neighbors\_swap]$(G.n)$\
    #h(14pt)$Delta arrow.l$ #text(fill: rgb("#1a5fa8"), weight: "bold")[incremental\_cost]$(G, y, a, b)$\
    #h(14pt)*if* $Delta <= 0$ *or* rand$() < e^(-Delta\/T)$:\
    #h(28pt)swap $y_a, y_b$; $C += Delta$\
    #h(28pt)*if* $C < C_"best"$:\
    #h(42pt)$C_"best" arrow.l C$; $quad y_"best" arrow.l y$\
    #h(14pt)$T arrow.l alpha dot T$\
    *return* $y_"best"$
  ]
  #pseudo3
][
  #v(4pt)
  *Custo incremental — `incremental_cost`*

  #v(8pt)
  Após o swap de $a$ e $b$, somente arestas incidentes a esses nós alteram sua contribuição ao custo. O restante cancela:

  $ Delta = sum_(j in N(a) union N(b)) Delta_j $
  $ O(deg(a) + deg(b)) "por iteração" quad (<< O(|E|)) $

  #v(8pt)
  *Critério de Metropolis:*
  - $Delta <= 0$: aceitar sempre (melhora)
  - $Delta > 0$: aceitar com $P = e^(-Delta \/ T)$
]


// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 12 — Validação: Gap de Otimalidade
// ─────────────────────────────────────────────────────────────────────────────
= Validação

== Gap de otimalidade vs. MILP

#grid(columns: (1.5fr, 1fr), gutter: 20pt, align: horizon)[
  #image("../figures/optimality_gap.png", width: 100%)
][
  *Metodologia:*
  $ "Gap"(%) = frac(C_"SA" - C_"MILP", C_"MILP") times 100 $

  18 instâncias verificadas ($n <= 12$, `solve_time < 250 s`)

  #v(10pt)
  *Resultados:*
  - *17/18* instâncias com gap = 0%
  - 1 instância: gap = +4% (n=10, seed=1)
  - avg gap = #hl[0,22%]
  - max gap = #hl[4,00%]

  #v(8pt)
  Muito abaixo da #note[meta de 10%] definida no plano do projeto.
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 13 — Aplicação Real: BTreeFy
// ─────────────────────────────────────────────────────────────────────────────
= Aplicação Real

== Fechando o ciclo: do modelo ao código embarcado

#grid(columns: (1fr, 1fr), gutter: 24pt)[
  *Pipeline `btf_layout_emitter.py`:*

  #v(8pt)
  1. Parseia modelo XML (BT)
  2. Constrói `BTGraph` (LCRS)
  3. Executa `solve()` com $alpha = 0.995$
  4. Remapeia índices parent/child/sibling por $y^*$
  5. Emite `btf_nodes_generated.c` drop-in

  #v(10pt)
  ```bash
  python3 btf_layout_emitter.py \
    -m models/porta_automatica.xml \
    --alpha 0.995 --seed 42
  ```
][
  #tag[Resultados em BTs reais]
  #v(10pt)
  #table(
    columns: (auto, auto, auto, auto),
    stroke: 0.5pt + rgb("#cccccc"),
    fill: (_, row) => if row == 0 { rgb("#f0f4fa") } else { white },
    inset: 7pt,
    [*BT*], [*n*], [*DFS*], [*SA*],
    [PortaAutomatica], [20], [95], hl[71],
    [AssetTracking],   [ 6], [16], hl[12],
  )

  #v(10pt)
  Reduções de *25%* e *18%* no custo MinLA.

  #v(10pt)
  `./compile-execute.sh --with-tests` → #hl[BUILD & TESTS PASSED ✓]

  #note[A semântica da árvore é preservada — apenas a ordem do array muda.]
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 14 — Conclusão
// ─────────────────────────────────────────────────────────────────────────────
= Conclusão

== Contribuições e limitações

#grid(columns: (1fr, 1fr), gutter: 24pt)[
  *Contribuições:*

  - *P1 — MILP:* formulação exata; ótimo em 0,19 s para $n=5$; 33% de redução vs. DFS

  - *P2 — Scaling:* muro exponencial empírico documentado em $n approx 12$–$15$

  - *P3 — SA:* $alpha = 0.995$ + init Fiedler; avg gap = 0,22% vs. MILP ótimo

  - *P4 — Validação:* emissor C funcional; build e testes passando com layout otimizado
][
  *Limitações e trabalhos futuros:*

  - MinLA é um *proxy* de localidade de cache — não minimiza diretamente cache misses

  - Pesos $W_(i j)$ uniformes ($=1$); extensão natural: pesos guiados por profiling

  - SA não escalonado para $n >> 200$ sem calibração adicional de $T_0$

  - Potencial de melhoria: simetria breaking no MILP, heurísticas construtivas alternativas
]

// ─────────────────────────────────────────────────────────────────────────────
// SLIDE 15 — Referências
// ─────────────────────────────────────────────────────────────────────────────
= Referências

#v(16pt)
#set text(size: 15pt)

- *Garey, M. R. & Johnson, D. S. (1979).* _Computers and Intractability: A Guide to the Theory of NP-Completeness._ Freeman. — Prova de NP-Dificuldade do MinLA.

#v(6pt)
- *Pettis, K. & Hansen, R. C. (1990).* Profile guided code positioning. _ACM SIGPLAN Notices_, 25(6), 16–27. — Princípio de localidade espacial; base teórica do vetor de Fiedler.

#v(6pt)
- *Kirkpatrick, S., Gelatt, C. D. & Vecchi, M. P. (1983).* Optimization by Simulated Annealing. _Science_, 220(4598), 671–680. — Base teórica do SA: critério de Metropolis e cooling schedule.

#v(6pt)
- *Petit, J. (2011).* Experiments on the minimum linear arrangement problem. _Journal of Experimental Algorithmics_, 8. — Benchmark de escalabilidade do MinLA; metodologia de gap de otimalidade.