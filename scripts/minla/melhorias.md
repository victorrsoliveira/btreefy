# Melhorias no algoritmo

## Temperatura inicial

The most highly recommended simple approach is the Delta-Based Acceptance Method. Instead of looking at the total initial cost, this method sets the initial temperature based on the average cost increase (\(\Delta E\)) of your initial moves. It ensures that the algorithm begins with a predictable, mathematically sound probability of accepting worse solutions.

### Step-by-Step Implementation

To calculate this initial temperature, follow these steps:
   1. Sample Random Moves: Generate \(m\) random neighboring solutions (typically \(m = 50\) to \(100\)) from your starting state.
   2. Filter Cost Increases: Identify only the moves that worsen the cost (\(\Delta E > 0\)).
   3. Calculate Average Increase: Find the average value of these worsening moves, denoted as \(\Delta E_{avg}\).
   4. Choose Target Probability: Select an initial acceptance probability (\(P_{0}\)) for these bad moves, typically between \(0.8\) and \(0.9\) (\(80\%\) to \(90\%\)).
   5. Solve for Temperature: Use the inverted Boltzmann probability formula to find the initial temperature (\(T_{0}\)):\(T_{0}=-\frac{\Delta E_{avg}}{\ln (P_{0})}\)

### Implementation Example

If your average worsening move increases the cost by \(\Delta E_{avg} = 50\), and you want a \(80\%\) acceptance rate (\(P_0 = 0.8\)):
\(T_{0}=-\frac{50}{\ln (0.8)}\approx -\frac{50}{-0.2231}\approx 224.1\)

Setting \(T_0 \approx 224\) ensures your algorithm behaves exactly as intended during the critical early iterations, regardless of whether your total initial cost is \(1,000\) or \(1,000,000\).

3.Comparison of Simple Methods
Method Implementation Effort Reliability Core Risk Delta-Based Low (Requires a brief sampling loop)
**High Slightly** increases initial setup time
**Initial Cost Zero** Very Low Random walk or immediate freezing 
**Fixed Constant Zero** Low Requires blind guessing per problem

### ✅ Final Recommendation
The Delta-Based Acceptance Method is the standard industry practice for a reliable, simple setup. It completely decouples the temperature from the absolute scale of your cost function, focusing instead on the scale of the changes between solutions.If you want, you can share your programming language or how you calculate a neighbor solution, and I can provide the exact code snippet to automate this setup for you.