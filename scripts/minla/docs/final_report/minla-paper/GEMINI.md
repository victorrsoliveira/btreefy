# GEMINI.md — Contexto do Artigo Científico: MinLA em Behavior Trees

Este arquivo é a **única fonte de verdade (Single Source of Truth)** para a redação do artigo científico utilizando o Antigravity CLI. O agente deve ler este arquivo no início de cada sessão e atualizá-lo a cada mudança estrutural relevante.

---

## 1. Metadados do Projeto

| Campo | Valor |
|---|---|
| **Título** | O problema de Minimum Linear Arrangement em Behavior Trees: uma solução com Simulated Annealing |
| **Autor** | Victor R. S. de Oliveira |
| **Afiliação** | PPGI — Universidade Federal de Alagoas (UFAL) |
| **Idioma** | Português (pt-BR) |
| **Público-alvo** | Professores universitários / banca de pós-graduação |
| **Tom** | Simples, claro e academicamente rigoroso |
| **Template Typst** | `charged-ieee` (`@preview/charged-ieee:0.1.4`) |
| **Arquivo principal** | `main.typ` |
| **Bibliografia** | `refs.bib` |
| **Compilador** | Typst 0.15.0 |
| **Comando de compilação** | `typst compile --font-path ./fonts --root ../.. main.typ` (executar de `minla-paper/`) |
| **Hardware-alvo** | ARM Cortex-M (Cortex-M0 e Cortex-M4 disponíveis para teste) |
| **Escopo** | Formalização do problema MinLA em BTs + prova de conceito da meta-heurística SA |

---

## 2. Referência de Conteúdo

O conteúdo técnico e os resultados experimentais provêm exclusivamente de:

- **Apresentação base:** `../presentation.typ` — fonte canônica de figuras, dados e estrutura lógica.
- **Contexto da apresentação:** `../GEMINI.md` — ground truth dos resultados por fase (P1–P4), decisões de design e parâmetros do solver.

### Fichamento dos Resultados (Ground Truth)

| Fase | Deliverable | Resultado-chave |
|------|-------------|------------------|
| **P1 — MILP** | Solver exato (PuLP/CBC → **CPLEX_PY em andamento**) | Custo ótimo = 8 (toy, 5 nós); 0,19 s; redução de 33% vs. DFS |
| **P2 — Scaling** | Muro exponencial | CBC: intratável a partir de n ≈ 12–15. **CPLEX: barreira em n ≥ 20 (preliminar, aguardando conclusão)** |
| **P3 — SA** | SA + init espectral; análise α | α=0.995 recomendado; gap 0% em 23/25; gap máx = 7,14%; avg_gap = 0,48% |
| **P4 — Validação** | Gap + emissor C | PortaAutomatica (n=20): DFS=95 → SA=71 (−25%); build e testes OK |

> ✅ **MILP CPLEX concluído (2026-07-01):** Experimento validou o avanço da fronteira de intratabilidade para n ≥ 20. O conjunto de instâncias ótimas conhecidas saltou de 18 (com CBC) para 25 (com CPLEX, todas as instâncias n ≤ 15). O artigo e as figuras foram integralmente atualizados com a amostra completa.

> ⛔ **AssetTracking DESCARTADO:** Os dados do modelo AssetTracking estavam incorretos e não serão citados no artigo. A seção de aplicação real usa apenas PortaAutomatica.

### Nota Crítica sobre Cache (preservar no artigo)
MinLA minimiza distância estrutural (proxy de localidade), mas **não garante** menos misses em caches direct-mapped de poucas linhas — o DFS pré-ordem tem prefetching sequencial natural. Isso é uma limitação a expor honestamente.

---

## 3. Estrutura do Artigo (Seções Planejadas)

```
Resumo (Abstract)
1. Introdução
2. Trabalhos Relacionados
3. Formulação do Problema
4. Metodologia
   4.1 Abordagem Exata: MILP
   4.2 Meta-heurística: Simulated Annealing
       4.2.1 Inicialização Espectral (Fiedler)
       4.2.2 Temperatura Inicial Adaptativa
       4.2.3 Geração de Vizinhança e Custo Incremental
       4.2.4 Critério de Aceitação e Resfriamento
5. Resultados e Discussão
   5.1 Análise de Escalabilidade do MILP
   5.2 Qualidade do SA: Gap de Otimalidade
   5.3 Aplicação em BTs Reais
6. Conclusão
Referências
```

---

## 4. Referências Bibliográficas (a incluir em `refs.bib`)

| Chave | Referência |
|-------|-----------|
| `garey1979` | Garey & Johnson (1979). *Computers and Intractability.* Freeman. |
| `pettis1990` | Pettis & Hansen (1990). Profile guided code positioning. *ACM SIGPLAN Notices*, 25(6), 16–27. |
| `kirkpatrick1983` | Kirkpatrick, Gelatt & Vecchi (1983). Optimization by Simulated Annealing. *Science*, 220(4598), 671–680. |
| `petit2011` | Petit, J. (2011). Experiments on the minimum linear arrangement problem. *J. Experimental Algorithmics*, 8. |
| `iovino2022` | Iovino, M. et al. (2022). A survey of Behavior Trees in robotics and AI. *Robotics and Autonomous Systems*, 154. |
| `ogren2012` | Ögren, P. (2012). Increasing modularity of UAV control systems using computer game behavior trees. *AIAA GNC Conference*. *(a confirmar — referência seminal sobre BTs em sistemas embarcados/autônomos)* |

> ⚠️ **TODO:** Verificar DOIs e dados completos antes da versão final.  
> ⚠️ **TODO (author):** Pesquisar nas bases IEEE Xplore / ACM DL / Google Scholar com as strings abaixo para enriquecer a seção de Trabalhos Relacionados.

### Strings de busca sugeridas para Trabalhos Relacionados
```
1. "behavior tree" AND ("embedded" OR "microcontroller" OR "RTOS") AND ("memory" OR "cache")
2. "minimum linear arrangement" AND ("heuristic" OR "metaheuristic") AND ("tree" OR "graph")
3. "data layout optimization" AND ("control flow" OR "decision tree") AND ("cache performance")
4. "profile guided" AND ("code layout" OR "data layout") AND "embedded"
5. "behavior tree" AND ("Zephyr" OR "ARM Cortex" OR "FreeRTOS") AND "optimization"
```
**Bases:** IEEE Xplore, ACM Digital Library, Google Scholar, Semantic Scholar.

---

## 5. Figuras Disponíveis

As figuras ficam em `../figures/` (relativo a `docs/`) e são geradas por `gen_figures.py`:

| Arquivo | Uso no artigo |
|---------|---------------|
| `scaling_wall.png` | Seção 5.1 — Análise de escalabilidade MILP |
| `optimality_gap.png` | Seção 5.2 — Gap de otimalidade SA vs. MILP |

---

## 6. Diretrizes de Escrita

- **Tom:** Direto e objetivo. Evitar jargão excessivo; definir termos técnicos na primeira ocorrência.
- **Matemática:** Sempre em blocos Typst (`$ ... $`). Nunca deixar valores absolutos sem linearização no contexto MILP.
- **Dados:** Nunca afirmar resultados sem respaldo nos dados das Fases P1–P4.
- **Limitações:** Expor honestamente (cache miss, pesos uniformes, n ≤ 31 testado).
- **Citações:** Todo claim não-trivial deve ter referência. Usar chaves `@garey1979`, `@pettis1990`, etc.
- **Comprimento:** Artigo curto estilo IEEE (4–6 páginas A4, 2 colunas). Ser conciso.

---

## 7. Fluxo de Trabalho Iterativo (Instruções para o Agente)

> ⚠️ **CRÍTICO:** Atualizar este arquivo a cada mudança estrutural (nova seção finalizada, decisão de design, resultado adicionado).

**Para cada seção:**
1. **Perguntar** ao autor as informações ausentes antes de escrever.
2. **Propor** estrutura em tópicos para validação.
3. **Escrever** o conteúdo em Typst (`main.typ`).
4. **Compilar:** `typst compile main.typ` — validar sintaxe.
5. **Criticar:** apontar lacunas de citação ou justificativa quantitativa.

---

## 8. Estado Atual do Projeto

| Item | Estado |
|------|--------|
| `GEMINI.md` (este arquivo) | ✅ Criado |
| `main.typ` (esqueleto template) | ⚠️ Contém conteúdo placeholder do charged-ieee |
| `refs.bib` | ⚠️ Contém referências placeholder |
| Resumo | ❌ Não escrito |
| Introdução | ❌ Não escrita |
| Trabalhos Relacionados | ❌ Não escrito |
| Formulação do Problema | ❌ Não escrita |
| Metodologia | ❌ Não escrita |
| Resultados e Discussão | ❌ Não escrito |
| Conclusão | ❌ Não escrita |

---

## 9. Dados Verificados e Aprovados para o Artigo

Todos os números abaixo foram auditados e estão aprovados para uso no artigo:

| Número | Origem | Status |
|--------|--------|--------|
| **33%** de redução (toy example) | DFS=12 → MILP=8; (12−8)/12 | ✅ Verificado |
| **25%** de redução (PortaAutomatica) | DFS=95 → SA=71; (95−71)/95 | ✅ Verificado |
| **avg_gap = 0,22%** | Média de 18 instâncias SA vs. MILP (n≤12) | ✅ Verificado |
| **17/18** instâncias com gap = 0% | Fase P3 | ✅ Verificado |
| **gap máx = 4%** (Δ=1, n=10) | 1 instância fora do ótimo | ✅ Verificado |
| PortaAutomatica: n=20, DFS=95, SA=71 | Fase P4 | ✅ Verificado |

> ⛔ **AssetTracking DESCARTADO** (decisão de 2026-06-30): dados inconsistentes, exemplo removido do artigo. O artigo não faz referência a este modelo.

### Narrativa quantitativa central do artigo
1. O MILP garante a solução ótima, mas torna-se intratável para n ≥ 15.
2. O SA com inicialização espectral atinge avg_gap = 0,22% — praticamente ótimo.
3. Em uma BT real de 20 nós (PortaAutomatica), o SA reduz o custo MinLA em 25% vs. DFS.
