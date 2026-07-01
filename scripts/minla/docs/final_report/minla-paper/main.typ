// =============================================================================
// main.typ — O problema de Minimum Linear Arrangement em Behavior Trees
//            Uma solução com Simulated Annealing
// Autor: Victor R. S. de Oliveira
// PPGI — Universidade Federal de Alagoas (UFAL)
// Template: charged-ieee (Typst)
// =============================================================================
// COMPILAR DE: scripts/minla/docs/final_report/minla-paper/
// COMANDO:     typst compile --font-path ./fonts --root ../.. main.typ
// FIGURAS EM:  ../../figures/ (relativo a este arquivo)
// =============================================================================

#import "@preview/charged-ieee:0.1.4": ieee

#show: ieee.with(
  title: [O problema de _Minimum Linear Arrangement_ em Behavior Trees:
           uma solução com Simulated Annealing],
  abstract: [
    Em processadores ARM Cortex-M, Behavior Trees (BTs) são percorridas por
    busca em profundidade sobre um vetor plano de structs em C: nós conectados
    por ponteiros inteiros distantes no array forçam acessos não-contíguos à
    memória, fenômeno conhecido como _pointer chasing_, que degrada o
    desempenho em dispositivos com cache restrito. Este artigo formula o
    problema de ordenação ótima dos nós como uma instância do Minimum Linear
    Arrangement (MinLA) — NP-Difícil no caso geral — e propõe uma solução em
    dois estágios. Primeiro, um solver de Programação Linear Inteira Mista
    (MILP) determina o layout ótimo exato para instâncias pequenas, servindo
    como referência de otimalidade. Segundo, uma meta-heurística de Simulated
    Annealing, inicializada pelo vetor de Fiedler do grafo LCRS para maximizar
    a localidade espacial desde o ponto de partida, é aplicada a instâncias de
    tamanho real. O SA atinge gap médio de 0,22% em relação ao ótimo MILP, com
    17 das 18 instâncias de benchmark alcançando o valor ótimo exato. Em uma
    Behavior Tree real de 20 nós, o método reduz o custo MinLA em 25% em
    relação ao layout padrão gerado por DFS pré-ordem.
  ],
  authors: (
    (
      name: "Victor R. S. de Oliveira",
      department: [Programa de Pós-Graduação em Informática],
      organization: [Universidade Federal de Alagoas],
      location: [Maceió, AL, Brasil],
      // TODO: confirmar e-mail institucional
      email: "victor@ic.ufal.br"
    ),
  ),
  index-terms: (
    "Behavior Trees",
    "Minimum Linear Arrangement",
    "Simulated Annealing",
    "Sistemas Embarcados",
    "Localidade de Cache",
  ),
  bibliography: bibliography("refs.bib"),
  figure-supplement: [Figura],
)

#set text(lang: "pt")

// =============================================================================
// SEÇÃO 1 — INTRODUÇÃO
// =============================================================================
= Introdução

Behavior Trees (BTs) são formalismos de controle que se tornaram amplamente
adotados em robótica e sistemas de IA por oferecerem maior modularidade e
reatividade do que autômatos finitos @iovino2022. A crescente adoção de BTs em
plataformas embarcadas de baixo custo — como microcontroladores ARM Cortex-M —
introduz restrições de desempenho que ainda são pouco estudadas na literatura.

O framework BTreeFy implementa BTs em C para ambientes bare-metal e RTOS,
armazenando os nós como um vetor plano (array) de structs do tipo `btf_node`.
Cada nó mantém índices inteiros para seu pai, filho esquerdo e irmão seguinte,
seguindo a representação Left-Child Right-Sibling (LCRS):

```c
struct btf_node {
    uint32_t parent;
    uint32_t child;
    uint32_t sibling;
    btf_action_fn_t  action;
    btf_control_fn_t control;
};
```

A execução da BT percorre esses índices a cada ciclo de controle por meio de
uma travessia em profundidade (DFS). Em processadores com cache de dados
estreito ou memória de acesso lento — como nos Cortex-M0 sem TCM, ou mesmo
em Cortex-M4 com caches de pequena capacidade — nós conectados por arestas
LCRS que ocupam posições distantes no array causam o fenômeno de
_pointer chasing_: o processador precisa carregar regiões de memória não
contíguas a cada salto de travessia, impondo latências adicionais.

Este trabalho investiga a seguinte questão: _dada uma Behavior Tree com $n$
nós, qual é a permutação do vetor de nós que minimiza a soma das distâncias
entre nós adjacentes durante a travessia?_ Essa questão é formalizada como o
problema de Minimum Linear Arrangement (MinLA) @garey1979.

As principais contribuições deste artigo são:
- A formalização do problema de layout de BTs como instância de MinLA.
- A demonstração empírica da fronteira de intratabilidade do solver MILP.
- Um algoritmo SA com inicialização espectral (vetor de Fiedler) que atinge
  gap médio de 0,22% em relação ao ótimo em instâncias de benchmark.
- Um emissor C integrado ao BTreeFy que automatiza a geração do vetor
  reordenado como arquivo drop-in.

O restante deste artigo está organizado como segue. A Seção~II discute
trabalhos relacionados. A Seção~III apresenta a formulação matemática do
problema. A Seção~IV descreve a metodologia de solução. A Seção~V apresenta
os resultados experimentais. A Seção~VI conclui o trabalho.

// =============================================================================
// SEÇÃO 2 — TRABALHOS RELACIONADOS
// =============================================================================
= Trabalhos Relacionados

O problema MinLA foi provado NP-Difícil para grafos gerais por Garey e
Johnson @garey1979. Petit @petit2011 conduziu experimentos sistemáticos de
benchmark, caracterizando o comportamento de heurísticas e solvers exatos
para instâncias com dezenas a centenas de vértices e estabelecendo métricas
de gap de otimalidade amplamente adotadas.

O uso do vetor de Fiedler — segundo menor autovetor do Laplaciano do grafo —
como critério de ordenação para maximizar localidade espacial foi proposto por
Pettis e Hansen @pettis1990 no contexto de posicionamento de código guiado por
perfil de execução. Os autores demonstraram que ordenar funções pela componente
do vetor de Fiedler minimiza um proxy do número de transições de página durante
a execução, princípio diretamente análogo ao objetivo do MinLA.

O Simulated Annealing como meta-heurística para problemas combinatoriais NP-
Difíceis foi introduzido por Kirkpatrick et al. @kirkpatrick1983, que
estabeleceram o critério de aceitação de Metropolis e o esquema de
resfriamento geométrico. O SA tem sido amplamente aplicado a problemas de
roteamento, agendamento e otimização de layout.

No domínio de Behavior Trees, Iovino et al. @iovino2022 apresentam uma revisão
abrangente de formalismos, algoritmos de síntese e aplicações em robótica e
IA. Ögren @ogren2012 estabeleceu BTs como formalismo de controle para sistemas
autônomos modulares. Nenhum dos trabalhos revisados aborda o problema de
otimização de layout em memória para BTs em sistemas embarcados. Este trabalho
preenche essa lacuna ao conectar o referencial teórico do MinLA com a
representação LCRS do BTreeFy.

// =============================================================================
// SEÇÃO 3 — FORMULAÇÃO DO PROBLEMA
// =============================================================================
= Formulação do Problema

== Modelo de Grafo LCRS

A partir de um vetor de $n$ nós `btf_node[]`, constrói-se o grafo não-dirigido
ponderado $G = (V, E, W)$ da seguinte forma:

- $V = {0, 1, ..., n-1}$: um vértice por nó do array.

- $E$: para cada nó $i$, inserem-se arestas $(i, "node"[i]."parent")$,
  $(i, "node"[i]."child")$ e $(i, "node"[i]."sibling")$ quando o campo
  correspondente é não-nulo. Arestas duplicatas são removidas. Para uma BT com
  representação LCRS, $|E| = O(n)$.

- $W_(i j) = 1$ para toda aresta (pesos uniformes como linha de base; extensão
  com pesos guiados por profiling é trabalho futuro).

== Definição do Problema MinLA

Seja $y : V -> {0, 1, ..., n-1}$ uma bijeção que atribui cada nó a um slot
único do array. O problema MinLA consiste em encontrar $y^*$ que minimize:

$ min_y quad sum_((i,j) in E) W_(i j) dot |y_i - y_j| $ <eq:minla>

$ "sujeito a:" quad y_i != y_j quad forall i != j $

A quantidade $|y_i - y_j|$ representa a distância entre os nós $i$ e $j$ na
memória linear. Minimizar a soma ponderada dessas distâncias equivale a
posicionar nós conectados o mais próximo possível no vetor.

MinLA é NP-Difícil para grafos gerais @garey1979. Embora BTs sejam árvores
esparsas, a representação LCRS introduz arestas de irmão que tornam o grafo
não puramente hierárquico; a extensão a pesos não-uniformes reintroduz a
dificuldade computacional.

== Exemplo Ilustrativo

Considera-se o BT $"Fallback" -> {"Sequence" -> {A, B}, C}$ com $n = 5$ nós.
As arestas LCRS formam $E = {(0,1),(0,4),(1,2),(1,3),(1,4),(2,3)}$, com seis
arestas. A @tab:toy compara os dois layouts extremos.

#figure(
  placement: top,
  table(
    columns: (auto, auto, auto),
    align: (left, center, center),
    inset: (x: 8pt, y: 5pt),
    stroke: 0.5pt + rgb("#cccccc"),
    fill: (_, row) => if row == 0 { rgb("#f0f4fa") } else { white },
    table.header[*Layout*][*Ordem no array*][*Custo MinLA*],
    [DFS pré-ordem (padrão)], [`[F, S, A, B, C]`], [12],
    [Ótimo (MILP)],           [`[C, F, S, B, A]`], [*8*],
  ),
  caption: [Comparação de layouts para o exemplo ilustrativo de 5 nós.
            A reordenação reduz o custo MinLA em 33%.],
) <tab:toy>

O layout ótimo, encontrado pelo MILP, reduz o custo em 33% em relação ao
layout padrão gerado pelo parser DFS, apenas reordenando o array —
sem qualquer alteração na semântica da BT.

// =============================================================================
// SEÇÃO 4 — METODOLOGIA
// =============================================================================
= Metodologia

== Abordagem Exata: MILP

Para eliminar o valor absoluto em @eq:minla, introduz-se a variável auxiliar
$d_(i j) >= 0$ por aresta, com as restrições:

$ d_(i j) >= y_i - y_j, quad d_(i j) >= y_j - y_i quad forall (i,j) in E $
<eq:dij>

Um solver minimizador conduz $d_(i j)$ ao valor exato $|y_i - y_j|$. A função
objetivo torna-se $min sum_(i,j) W_(i j) d_(i j)$, que é linear.

Para impor a unicidade dos slots ($y_i != y_j$), introduz-se uma variável
binária $z_(i j) in {0,1}$ para cada par $i != j$, com Big-M $= n$:

$ y_i - y_j >= 1 - n z_(i j) $ <eq:bigm1>
$ y_j - y_i >= 1 - n (1 - z_(i j)) quad forall i != j $ <eq:bigm2>

O modelo completo possui $O(n)$ variáveis contínuas — $y_i$ e $d_(i j)$ — e
$O(n^2)$ variáveis binárias — $z_(i j)$. O crescimento quadrático no número
de variáveis binárias constitui o gargalo computacional do MILP, impondo uma
fronteira prática de intratabilidade conforme discutido na Seção~V.

// TODO (APÓS CPLEX): Inserir aqui a tabela de tempo de execução por n
// com os resultados do CPLEX_PY. Estrutura esperada:
// n | Vars. binárias z_ij | Tempo médio | Status
// 5 | 20  | < 1 s    | ótimo
// ...
// 20| 380 | timeout  | —
// A fronteira preliminar com CPLEX é n >= 20.

== Simulated Annealing com Inicialização Espectral

Para instâncias além do limite de intratabilidade do MILP, utiliza-se um
algoritmo de Simulated Annealing (SA) @kirkpatrick1983. O SA opera sobre o
espaço de permutações $cal(S)_n$ e é composto por quatro componentes
descritos a seguir.

=== Inicialização Espectral

A solução inicial $y^0$ é obtida a partir do vetor de Fiedler do grafo $G$,
seguindo o princípio de Pettis e Hansen @pettis1990:

+ Constrói-se o Laplaciano ponderado $L = D - A$, onde $D$ é a matriz de
  graus e $A$ é a matriz de adjacência.
+ Calculam-se os autovetores de $L$ via decomposição espectral.
+ O vetor de Fiedler é o autovetor correspondente ao segundo menor autovalor
  $lambda_2 > 0$.
+ Os nós são ordenados pela componente do vetor de Fiedler, produzindo $y^0$.

Essa inicialização posiciona a busca próximo a uma solução de boa qualidade,
reduzindo o espaço efetivo a explorar. No exemplo de 5 nós, a inicialização
espectral produziu diretamente o layout ótimo (custo 8), sem necessidade de
iterações SA adicionais.

=== Temperatura Inicial Adaptativa

A temperatura inicial $T_0$ é calibrada para garantir 80% de aceitação de
movimentos de piora no início da busca, evitando que a busca fique presa em
mínimos locais prematuramente. Realiza-se uma caminhada aleatória de 100 swaps
a partir de $y^0$, coletando apenas os deltas positivos $Delta^+ > 0$.
Com a média $overline(Delta)^+$, aplica-se a fórmula de Kirkpatrick
@kirkpatrick1983:

$ T_0 = -(overline(Delta)^+) / (ln(p_0)), quad p_0 = 0.8 $ <eq:t0>

=== Geração de Vizinhança e Custo Incremental

A cada iteração, sorteiam-se aleatoriamente dois nós distintos $(a, b)$ para
trocar de posição no vetor $y$. Após o swap, somente as arestas incidentes a
$a$ e $b$ alteram sua contribuição ao custo. O custo incremental $Delta$ é
calculado em $O("deg"(a) + "deg"(b))$, em vez de $O(|E|)$:

$ Delta = sum_(j in N(a) union N(b)) Delta_j $ <eq:delta>

=== Critério de Aceitação e Resfriamento

O critério de aceitação de Metropolis @kirkpatrick1983 define:

$ P("aceitar") = cases(
  1 & "se" Delta <= 0,
  e^(-Delta \/ T) & "se" Delta > 0
) $ <eq:metropolis>

O resfriamento segue o esquema geométrico $T_(k+1) = alpha dot T_k$, com
parada em $T < 10^(-4)$. O valor $alpha = 0.995$ foi selecionado após análise
de sensibilidade com $alpha in {0.99, 0.995, 0.999}$, oferecendo o melhor
equilíbrio entre qualidade da solução e tempo de execução.

// =============================================================================
// SEÇÃO 5 — RESULTADOS E DISCUSSÃO
// =============================================================================
= Resultados e Discussão

== Análise de Escalabilidade do MILP

O MILP foi implementado com PuLP e executado com o solver CPLEX_PY sobre 30
instâncias aleatórias de BTs com $n in {5, 8, 10, 12, 15, 20}$ (5 instâncias
por valor de $n$) e limite de 300~s por instância. A @tab:milp resume os
resultados e a @fig:scaling ilustra o tempo de resolução em escala logarítmica.

#figure(
  placement: top,
  table(
    columns: (auto, auto, auto, auto),
    align: (center, center, right, left),
    inset: (x: 8pt, y: 5pt),
    stroke: 0.5pt + rgb("#cccccc"),
    fill: (_, row) => if row == 0 { rgb("#f0f4fa") } else { white },
    table.header[*$n$*][*Binárias $z_(i j)$*][*Tempo médio (s)*][*Status*],
    [5],  [20],  [0,06],  [Ótimo],
    [8],  [56],  [0,13],  [Ótimo],
    [10], [90],  [0,28],  [Ótimo],
    [12], [132], [1,73],  [Ótimo],
    [15], [210], [73,36], [Ótimo],
    [20], [380], [300,09],[*Timeout*],
  ),
  caption: [Tempo de resolução médio do MILP por tamanho de instância.
            Para $n = 20$, todas as 5 instâncias atingiram o limite de 300~s.],
) <tab:milp>

Para $n <= 15$, o CPLEX encontrou a solução ótima em todas as instâncias,
com tempo médio crescendo de $0{,}06$~s ($n=5$) para $73{,}4$~s ($n=15$) ---
aumento de aproximadamente três ordens de magnitude. Para $n = 20$, todas as
instâncias esgotaram o limite de 300~s, confirmando a fronteira prática de
intratabilidade do MILP em $n >= 20$. O comportamento exponencial do tempo de
resolução em função de $n$ reflete o crescimento $O(n^2)$ no número de
variáveis binárias $z_(i j)$, conforme esperado pela análise de complexidade.

#figure(
  image("../../figures/scaling_wall.png", width: 88%),
  placement: top,
  caption: [Tempo de resolução do MILP (CPLEX) em função do número de nós $n$.
            A curva em escala logarítmica evidencia o crescimento exponencial;
            a linha tracejada vermelha indica o limite de 300~s. Para $n = 20$,
            média e máximo coincidem no teto de timeout.],
) <fig:scaling>

== Qualidade do SA: Gap de Otimalidade

O gap de otimalidade entre SA e MILP é definido como:

$ "Gap"(%) = (C_"SA" - C_"MILP") / C_"MILP" times 100 $ <eq:gap>

O SA foi avaliado em 18 instâncias de BTs com $n <= 12$, para as quais o MILP
encontrou a solução ótima dentro do limite de tempo. A @fig:gap apresenta os
resultados individuais por instância.

#figure(
  image("../../figures/optimality_gap.png", width: 88%),
  placement: top,
  caption: [Gap de otimalidade do SA em relação ao MILP para 18 instâncias
            com $n <= 12$. Instâncias em verde atingiram o ótimo exato;
            a instância em vermelho apresentou desvio de 4%.],
) <fig:gap>

O SA com inicialização espectral atingiu o valor ótimo exato em 17 das 18
instâncias (gap = 0%). A única instância com desvio, com $n = 10$, apresentou
gap de 4%, correspondente a $Delta = 1$ unidade de custo. O gap médio sobre
o conjunto completo foi de 0,22%, demonstrando que o SA opera praticamente no
nível do ótimo para o horizonte de instâncias avaliado.


== Aplicação em Behavior Tree Real

Para avaliar a aplicabilidade prática, o SA foi executado sobre o modelo
PortaAutomatica, com $n = 20$ nós. O layout padrão, produzido automaticamente
pelo parser `btf_groot_parser.py` em ordem DFS pré-ordem, apresentou custo
MinLA de 95. O layout otimizado pelo SA ($alpha = 0.995$, $"seed" = 42$)
apresentou custo 71, representando uma redução de *25%*, conforme
resumido na @tab:real.

#figure(
  placement: top,
  table(
    columns: (auto, auto, auto, auto),
    align: (left, center, center, center),
    inset: (x: 8pt, y: 5pt),
    stroke: 0.5pt + rgb("#cccccc"),
    fill: (_, row) => if row == 0 { rgb("#f0f4fa") } else { white },
    table.header[*Modelo*][*$n$*][*Custo DFS*][*Custo SA*],
    [PortaAutomatica], [20], [95], [*71*],
  ),
  caption: [Custo MinLA antes e após a otimização por SA na BT PortaAutomatica.
            A redução de 25% é obtida apenas reordenando o array de nós.],
) <tab:real>

O emissor `btf_layout_emitter.py` recebe a permutação $y^*$ gerada pelo SA,
remapeia os campos `parent`, `child` e `sibling` de cada nó e emite o arquivo
`btf_nodes_generated.c`, substituível diretamente no projeto BTreeFy. A
compilação e a execução dos testes unitários com
`./compile-execute.sh --with-tests` foram bem-sucedidas, confirmando que a
semântica da BT é integralmente preservada pelo reordenamento.

// =============================================================================
// SEÇÃO 6 — CONCLUSÃO
// =============================================================================
= Conclusão

Este artigo formalizou o problema de ordenação de nós em Behavior Trees como
uma instância de Minimum Linear Arrangement (MinLA) e investigou duas
abordagens de solução: um solver MILP exato e uma meta-heurística Simulated
Annealing com inicialização espectral pelo vetor de Fiedler.

O MILP garante a solução ótima para instâncias com $n <= 15$, mas torna-se
intratável a partir de $n >= 20$ (utilizando o solver CPLEX_PY com limite de
300~s por instância). O SA com inicialização espectral alcança gap médio de
0,22% em relação ao ótimo, atingindo o valor exato em 17 das 18 instâncias
avaliadas. Aplicado à BT PortaAutomatica de 20 nós, o SA produz redução de
25% no custo MinLA em comparação ao layout padrão DFS, sem alterar a semântica
da árvore.

Identificam-se as seguintes limitações: (i) MinLA é um proxy de localidade de
cache — não minimiza diretamente cache misses em modelos de cache direct-mapped
com poucas linhas, onde o prefetching sequencial do DFS pré-ordem pode ser
vantajoso; (ii) os pesos $W_(i j)$ são uniformes neste trabalho, sendo a
extensão com pesos guiados por profiling uma direção natural; (iii) o
comportamento do SA para BTs com mais de 31 nós permanece como questão em
aberto.

Como trabalho futuro, prevê-se: a avaliação em hardware real (ARM Cortex-M0 e
Cortex-M4) com contagem de ciclos; o uso de pesos não-uniformes derivados de
perfis de execução; e a investigação de operadores de vizinhança alternativos
ao swap (e.g., inserção, reversão de segmento) para melhorar a qualidade da
busca local.
