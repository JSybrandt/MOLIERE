HYPOTHESIS CLUSTERING
=====================

This directory includes a couple different methods for hypothesis clustering.

The main procedure is to compare a square symmetric set similarity matrix for each hypothesis.

A[i,j] = 2 * |Set\_i Intersect Set\_j| / (|Set\_i| * |Set\_j|)

We then apply NMFk to factor this matrix.

Because some of our matrices are computationally unfeasible to calculate using julia,
we also utilize mlpack\_nmf. The advantage here is that this package gains parallelism through openBLAS
and is more easily distributed to a cluster of machines.

Therefore, we supply mlpack\_finialize.jl to assemble the final nmf solutions and evaluate silhouettes.

