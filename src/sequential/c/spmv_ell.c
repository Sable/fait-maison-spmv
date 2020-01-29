#include "spmv_ell.h"
#include "config.h"

void spmv_ell(int *indices, MYTYPE *data, int N, int nc, MYTYPE *x, MYTYPE *y)
{
  int i, j;
  for(j = 0; j < nc ; j++){
    for(i = 0; i < N; i++){
      y[i] += data[j * N + i] * x[indices[j * N + i]];
    }
  }
}

