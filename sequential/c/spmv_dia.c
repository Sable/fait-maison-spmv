#include "spmv_dia.h"
#include "config.h"

void spmv_dia(int *offset, MYTYPE *data, int N, int nd, int stride, MYTYPE *x, MYTYPE *y)
{
  int i, k, n, istart, iend, index;

  for(i = 0; i < nd; i++){
    k = offset[i];
    index = 0;
    istart = (0 < -k) ? index = N-stride, -k : 0;   
    iend = (N-1 < N-1-k) ? N-1 : N-1-k;
    for(n = istart; n <= iend; n++){
      y[n] += (data[(size_t)i*stride+n-index] * x[n+k]);
    } 
  }
}

