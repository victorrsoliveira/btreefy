# Scaling Analysis Instances

This folder contains documentation of all 30 BT instances used in the Phase 2 MILP scaling study.

- **File:** [`instances.md`](instances.md)
- **Generator:** [`scratch/generate_instances_doc.py`](../../../../../.gemini/antigravity-cli/brain/1f0a5809-60d0-4e5b-93a3-569423286711/scratch/generate_instances_doc.py)

## Regenerating

```bash
python3 /home/victor/.gemini/.../scratch/generate_instances_doc.py
```

Or equivalently, reconstruct any single instance with:

```python
from minla.random_bt import generate
g = generate(n=10, seed=2)
```
