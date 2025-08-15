# FastSbR
Distributed approximation algorithms for sorting by reversals (SbR) problem.

### Run all experiments using:

```bash
./run.sh
```

which will:

1) Compile all files and create ``elements/`` and ``results`` directory by running ``prepare.sh``.
2) Generate random permutations under ``elements/`` directory with given seed by running ``generate.sh``.
3) Run MS4, FD4, MS2, FD2, SQ4, SQ2 algorithms for each permutation.
4) Save the results under ``stats_<SEED>.csv``.

These steps are repeated **10 times** with different seeds (from 1000 to 1009).

### Environment:

- OS: Ubuntu 22.04 LTS
- Compiler: GCC 11.4.0
- MPI: OpenMPI 4.0
