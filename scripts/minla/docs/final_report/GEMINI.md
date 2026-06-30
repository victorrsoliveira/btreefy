# Contexto do Projeto: Apresentação Otimização & Pesquisa Operacional

Este arquivo serve como a única fonte de verdade (Single Source of Truth) para o desenvolvimento da apresentação acadêmica utilizando o Antigravity CLI. O agente deve ler este arquivo no início de cada sessão e atualizá-lo sempre que houver mudanças estruturais.

---

## 1. Metadados do Projeto
- **Disciplina:** Otimização & Pesquisa Operacional (Pós-Graduação)
- **Título da Apresentação:** "O problema de Minimum Layout Arrangement em Behavior Trees: uma solução com Simulated Annealing"
- **Autor:** Victor R. S. de Oliveira
- **Idioma:** Português (pt)
- **Tecnologia Principal:** Typst (com o ecossistema de pacotes `touying` e o tema `university`)
- **Arquivo Alvo Principal:** `presentation.typ`

---

## 2. Escopo Técnico e Conteúdo
A apresentação deve abordar de forma elegante, minimalista e matemática os seguintes tópicos:
1. **Introdução e Motivação:** O problema de localidade de dados e saltos de memória (_pointer-chasing_) ao executar Árvores de Comportamento (_Behavior Trees_) em sistemas embarcados estruturados em memória.
2. **Formulação Matemática:** Modelagem do problema utilizando o conceito de Arranjo Linear Mínimo (*Minimum Linear Arrangement - MinLA*), com foco na minimização da distância ponderada entre nós adjacentes.
3. **Meta-heurística (Simulated Annealing):** Estrutura do algoritmo de Recozimento Simulado para resolver o problema:
   - Definição do Espaço de Estados (Permutações do vetor de nós).
   - Solução Inicial baseada em Busca em Profundidade (DFS).
   - Função de Custo / Energia (Função objetivo do MinLA).
   - Mecanismo de Transição (Troca aleatória / *Swap* de nós).
   - Critério de Aceitação de Metropolis ($P = e^(-(Delta E) / T)$) e esquema de resfriamento.

---

## 3. Diretrizes de Design e Formatação
- **Estilo:** Slides limpos, altamente escaneáveis, evitando blocos densos de texto. Priorizar tópicos estruturados (*bullet points*).
- **Matemática:** Utilizar blocos nativos de fórmulas do Typst (`$ ... $`) para equações, restrições e funções objetivo, garantindo rigor acadêmico visual.

---

## 4. Regras de Operação e Fluxo de Trabalho (Instruções para o Agente)

> ⚠️ **CRÍTICO - Atualização de Contexto:** Toda vez que uma alteração importante acontecer no projeto (ex: modificação na modelagem matemática, inserção de novas seções, refinamento do algoritmo ou decisões críticas de design), o agente **DEVE atualizar imediatamente este arquivo (`GEMINI.md`)** para refletir o novo estado atual, mantendo o histórico de progresso e as premissas alinhados.

- **Validação de Compilação:** Antes de encerrar qualquer tarefa ou rodada de alterações, o agente deve validar o layout e a sintaxe executando localmente o comando:
  ```bash
  typst compile presentation.typ
  ```
---

## 5. Implementação do Solver — `sa_solver.py`

- **Arquivo:** `scripts/minla/sa_solver.py`
- **Função principal (entry-point da solução):** `solve(graph, alpha, T_min, max_iter, seed, record_every) → SAResult`
- **Responsabilidade:** Resolve o problema MinLA sobre um `BTGraph` usando Simulated Annealing com inicialização espectral (vetor de Fiedler).

### Pipeline interno da função `solve`

1. **Inicialização espectral** (`fiedler_permutation`): constrói o Laplaciano ponderado L = D - A, computa o vetor de Fiedler (2º menor autovetor via `scipy.linalg.eigh`) e ordena os nós por componente → permutação inicial y^0.
2. **Temperatura inicial** (`sa_initial_temperature`): calibrada automaticamente para 80% de aceitação de movimentos de piora (amostragem de 100 swaps aleatórios na vizinhança de y^0).
3. **Loop SA** (resfriamento geométrico T_{k+1} = α · T_k, para em T < 1e-4):
   - Sorteia par aleatório (a, b), a ≠ b.
   - Calcula Δ via `incremental_cost_delta` — O(deg(a) + deg(b)).
   - Aceita se Δ ≤ 0 ou com probabilidade exp(-Δ/T) (critério de Metropolis).
   - Rastreia a melhor solução vista.
4. **Retorna** `SAResult` com: `permutation`, `cost`, `initial_cost`, `alpha`, `iterations`, `history`.

### Parâmetros recomendados (validados — Fase 3)
- `alpha = 0.995` — melhor trade-off qualidade/velocidade.
- `T_min = 1e-4` — critério de parada.
- `seed = 42` — reprodutibilidade.

### Função auxiliar de comparação
- `solve_random_init(...)` — mesmo algoritmo SA com permutação inicial aleatória (usada nos gráficos Fiedler vs. aleatório).

---

## 6. Resultados por Fase (Ground Truth para a Apresentação)

| Fase | Deliverable | Resultado-chave |
|------|-------------|-----------------|
| **P1 — MILP** | Solver exato (PuLP/CBC) | Custo ótimo = 8 no toy example (5 nós) em 0,19 s; redução de 33% vs. DFS |
| **P2 — Scaling** | Análise empírica do "muro exponencial" | CBC intratável a partir de n ≈ 12–15; 30 instâncias caracterizadas |
| **P3 — SA** | SA com init espectral; análise de sensibilidade α | α=0.995 recomendado; gap 0% em 17/18 instâncias; gap máx = 4% |
| **P4 — Validação** | Gap de otimalidade + simulação de cache + emissor C | avg_gap = 0,22%; build e testes passaram; emissor gera .c drop-in |

### Nota crítica sobre cache (Fase 4)
O layout SA minimiza o custo MinLA (distância estrutural), mas **não garante** menos misses de cache em configurações direct-mapped de 2 linhas — o DFS pré-ordem tem prefetching sequencial natural que o SA desfaz. MinLA é um *proxy* de localidade, não um minimizador direto de misses. Esse ponto deve ser exposto com clareza na seção de validação da apresentação.

---

## 7. Decisões de Design da Apresentação (sessão 2026-06-29)

- **MILP:** Slide resumido — formulação e intratabilidade, sem detalhamento de d_ij / z_ij.
- **Toy example:** Diagrama inline em Typst usando `cetz` (similar ao TikZ). Fallback: tabela estruturada se o diagrama visual não satisfizer.
- **Simulação de cache:** **REMOVIDA** da apresentação. Não será citada.
- **Limite de slides:** 15 slides (expansível se necessário).
- **Figuras/gráficos:** Gerados em Python (Matplotlib), estilo simples e limpo — foco nos dados, sem decoração excessiva. Gerados conforme necessidade.

---

## 8. Comando de Compilação Correto

O `--root` deve apontar para `docs/` para permitir acesso às figuras em `../figures/`:

```bash
# Executar de: scripts/minla/docs/final_report/
typst compile --root .. presentation.typ
```

As figuras da apresentação ficam em `scripts/minla/docs/figures/` e são referenciadas
no .typ como `"../figures/<nome>.png"`.

Para regenerar as figuras:
```bash
python3 gen_figures.py
```
