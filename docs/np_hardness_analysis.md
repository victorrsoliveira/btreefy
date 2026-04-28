# NP-Hardness of the Cache-Aware BT Memory Layout Problem

## 1. Problem Statement

The core optimization problem studied in this project is: given a Behavior Tree (BT)
modeled as a weighted graph $G = (V, E, W)$, find a bijection (linear arrangement)

$$\pi : V \to \{1, 2, \dots, |V|\}$$

that minimizes the total weighted edge span across the 1D memory array:

$$\min_{\pi} \sum_{(i,j) \in E} W_{ij} \cdot |\pi(i) - \pi(j)|$$

This is an instance of the **Weighted Minimum Linear Arrangement (MinLA)** problem [1].

---

## 2. Background: The Minimum Linear Arrangement Problem

The **Minimum Linear Arrangement (MinLA)** problem was first formulated by Harper [2]
in 1964 in the context of signal coding theory. It belongs to the broader family of
*graph layout problems* [1], which encompass any combinatorial problem where the goal
is to embed graph vertices onto a line (or path) while optimizing a linear cost function
derived from edge distances.

**Formal Definition (unweighted):** Given an undirected graph $G = (V, E)$, find a
bijection $\pi : V \to \{1, \dots, n\}$ minimizing:

$$\text{MinLA}(G) = \min_{\pi} \sum_{\{u,v\} \in E} |\pi(u) - \pi(v)|$$

The weighted variant, relevant here, generalizes by scaling each term:

$$\text{WMinLA}(G, W) = \min_{\pi} \sum_{(i,j) \in E} W_{ij} \cdot |\pi(i) - \pi(j)|$$

Both variants are computationally equivalent in terms of complexity class (polynomial
reductions between them are trivial).

---

## 3. NP-Hardness: Formal Result

### Theorem (Garey, Johnson & Stockmeyer, 1974 / 1976)

> The decision version of MinLA — "Given $G = (V, E)$ and integer $k$, does there
> exist a linear arrangement $\pi$ such that $\sum_{(u,v) \in E} |\pi(u) - \pi(v)| \le k$?"
> — is **NP-complete**.

This result was first announced in [3] and published in full in the landmark paper
*"Some simplified NP-complete graph problems"* [4]. The proof proceeds by a polynomial
reduction from **Minimum Bisection**, a problem that is itself NP-hard [4, 5].

The key insight is that minimizing edge length in a linear order is at least as hard as
deciding whether a graph can be evenly bisected with few crossing edges, a property
deeply linked to partitioning problems that encode satisfiability instances.

### Corollary (Even & Shiloach, 1975)

MinLA remains NP-hard even when $G$ is restricted to **bipartite graphs** [6]. This
strengthens the hardness result: the difficulty is not an artifact of dense or irregular
graphs but persists in simple, well-structured graph families.

### Standard Reference Catalogue Entry

The decision problem is catalogued as problem **GT40** in the canonical NP-completeness
reference *Computers and Intractability* by Garey and Johnson [5], firmly establishing
its membership in the NP-complete class.

---

## 4. Implications for the BT Problem Instance

### 4.1 Structural Properties of BTs That Do Not Help

A Behavior Tree is a rooted, directed, acyclic graph (a tree). One might hope that
restricting to trees admits a tractable special case. This hope is partially justified:

- **Unweighted trees:** MinLA on unweighted trees is solvable in polynomial time.
  Algorithms by Shiloach [7], Chung [8], and Goldberg & Klipker exist that solve the
  problem exactly on trees in $O(n^2)$ or $O(n \log n)$ time.

However, our problem departs critically from this tractable case in two ways:

1. **Edge weights derived from execution profiling.** The weights $W_{ij}$ encode
   traversal frequencies (how often the BT scheduler transitions between nodes $i$ and
   $j$). When profiling is taken into account, the problem is no longer equivalent to
   pure graph structure minimization; it corresponds to the **weighted** version on the
   tree's *traversal graph*, which includes back-edges and cross-edges arising from
   runtime control flow. This effectively transforms the input from a tree into a
   **general weighted graph**.

2. **Cross-subtree control flow.** Fallback, Parallel, and Decorator nodes in BTs
   create execution dependencies that cross subtree boundaries, introducing edges in the
   profiling graph that are not present in the BT's structural tree. The resulting
   instance is a general graph, not a tree, and hence the polynomial-time tree algorithms
   do not apply.

### 4.2 Complexity Classification of the Project's Problem

| Graph Class             | MinLA Complexity           | Applies to BT Project? |
|:------------------------|:---------------------------|:-----------------------|
| General graphs          | **NP-Hard** [3, 4]         | ✓ (after profiling)    |
| Bipartite graphs        | **NP-Hard** [6]            | —                      |
| Unweighted trees        | P (poly-time solvable) [7, 8] | ✗ (weights present)  |
| Weighted general graphs | **NP-Hard** (trivial reduction from unweighted) | ✓ |

**Conclusion:** The weighted MinLA instance produced by the BT profiling graph is
**NP-Hard**. No polynomial-time exact algorithm exists for the general case unless
$\text{P} = \text{NP}$.

---

## 5. Inapproximability

Beyond NP-hardness of exact optimization, MinLA is also hard to approximate well:

- There is no known **constant-factor approximation** algorithm for general graphs [9].
- MinLA does **not** admit a **Polynomial-Time Approximation Scheme (PTAS)** unless
  NP-complete problems can be solved in randomized sub-exponential time [9].
- Under the **Exponential Time Hypothesis (ETH)**, MinLA is inapproximable within some
  constant factor [9].
- The best known approximation algorithm achieves a ratio of
  $O\!\left(\sqrt{\log n}\,\log\log n\right)$ using Semi-Definite Programming (SDP)
  relaxations and "spreading metrics" [9, 10].

These inapproximability results provide the formal justification for why this project
must rely on heuristic and meta-heuristic approaches (e.g., Simulated Annealing,
Spectral Sequencing) rather than seeking a guaranteed near-optimal MILP solution at
scale.

---

## 6. Relation to Other NP-Hard Problems in the Family

MinLA belongs to a well-studied hierarchy of graph layout problems [1]. The table below
situates it relative to related problems encountered in systems and compiler optimization:

| Problem               | Cost Function                     | Complexity   |
|:----------------------|:----------------------------------|:-------------|
| **MinLA**             | $\sum_{(u,v)} |\pi(u) - \pi(v)|$  | NP-Hard [4]  |
| **Bandwidth**         | $\max_{(u,v)} |\pi(u) - \pi(v)|$  | NP-Hard [5, 11] |
| **Profile**           | $\sum_v \max_{u \in N(v)} |\pi(u) - \pi(v)|$ | NP-Hard [1] |
| **Minimum Cut Linear Arrangement (MCLA)** | $\max_{1 \le k \le n-1} \text{cut}(k)$ | NP-Hard [1] |
| **Optimal Linear Extension** | topological order minimizing inversions | NP-Hard |

The shared NP-hardness across this family reflects a fundamental intractability:
embedding graphs in one dimension while respecting an objective over edge distances
cannot, in general, be done efficiently.

---

## 7. Why MILP Is Necessary but Insufficient at Scale

MILP formulations of MinLA provide **exact solutions** for small instances (the
"Week 1" milestone of this project). The decision version's membership in NP implies
that the search space grows super-polynomially, and empirically, MILP solvers
(PuLP/Gurobi) encounter the **"exponential wall"** — a breakdown in tractability —
at roughly $n \approx 20$–30 nodes for dense graphs (the "Week 2" scaling analysis).

This is the formal complexity-theoretic basis for transitioning to heuristics in
Week 3: NP-hardness guarantees that MILP cannot scale, and inapproximability results
bound how close any efficient algorithm can hope to get to the true optimum.

---

## 8. Summary

> **The Cache-Aware BT Memory Layout problem is NP-Hard.**

It is an instance of the Weighted Minimum Linear Arrangement problem on the BT's
profiling graph (a general weighted graph). The NP-hardness result was established by
Garey, Johnson & Stockmeyer (1974/1976) via reduction from Minimum Bisection, and
extends to bipartite graphs (Even & Shiloach, 1975). The problem is further
inapproximable within a PTAS, making heuristic methods the only viable approach at
operational scale.

---

## References

[1] Díaz, J., Petit, J., & Serna, M. (2002). **A survey of graph layout problems.**
*ACM Computing Surveys*, 34(3), 313–356.
https://doi.org/10.1145/568522.568523

[2] Harper, L. H. (1964). **Optimal assignments of numbers to vertices.**
*Journal of the Society for Industrial and Applied Mathematics*, 12(1), 131–135.
https://doi.org/10.1137/0112012

[3] Garey, M. R., Johnson, D. S., & Stockmeyer, L. (1974). **Some simplified NP-complete
problems.** In *Proceedings of the 6th Annual ACM Symposium on Theory of Computing
(STOC '74)*, pp. 47–63. ACM.

[4] Garey, M. R., Johnson, D. S., & Stockmeyer, L. (1976). **Some simplified NP-complete
graph problems.** *Theoretical Computer Science*, 1(3), 237–267.
https://doi.org/10.1016/0304-3975(76)90059-1

[5] Garey, M. R., & Johnson, D. S. (1979). **Computers and Intractability: A Guide to
the Theory of NP-Completeness.** W. H. Freeman and Company.
(Problem GT40: Minimum Linear Arrangement)

[6] Even, S., & Shiloach, Y. (1975). **NP-completeness of several arrangement problems.**
Technical Report TR-43, Computer Science Department, Technion – Israel Institute of
Technology, Haifa.

[7] Shiloach, Y. (1979). **A minimum linear arrangement algorithm for undirected trees.**
*SIAM Journal on Computing*, 8(1), 15–32.
https://doi.org/10.1137/0208002

[8] Chung, F. R. K. (1988). **On optimal linear arrangements of trees.**
*Computers & Mathematics with Applications*, 10(1), 43–60.
https://doi.org/10.1016/0898-1221(84)90085-3

[9] Charikar, M., Hajiaghayi, M. T., Karloff, H., & Rao, S. (2010).
**$l_2^2$ spreading metrics for vertex ordering problems.**
*Algorithmica*, 56(4), 577–604.
https://doi.org/10.1007/s00453-008-9192-z

[10] Feige, U., & Lee, J. R. (2007). **An improved approximation ratio for the minimum
linear arrangement problem.** *Information Processing Letters*, 101(1), 26–29.
https://doi.org/10.1016/j.ipl.2006.07.009

[11] Garey, M. R., Graham, R. L., Johnson, D. S., & Knuth, D. E. (1978).
**Complexity results for bandwidth minimization.**
*SIAM Journal on Applied Mathematics*, 34(3), 477–495.
https://doi.org/10.1137/0134037

[12] Petit, J. (2011). **Addenda to the survey of layout problems.**
*Bulletin of the EATCS*, 105, 177–201.

[13] Pettis, K., & Hansen, R. C. (1990). **Profile-guided code positioning.**
In *Proceedings of the ACM SIGPLAN 1990 Conference on Programming Language Design and
Implementation (PLDI '90)*, pp. 16–27. ACM.
https://doi.org/10.1145/93542.93550
