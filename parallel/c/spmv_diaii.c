#include "spmv_diaii.h"
#include "config.h"

void spmv_diaii(int *offset, MYTYPE *data, int start_row, int end_row, int nd, int N, int stride, MYTYPE *x, MYTYPE *y)
{
  int i, k, n, istart, iend, index;

  for(i = 0; i < nd; i++){
    k = offset[i];
    index = 0;
    istart = (0 < -k) ? index = N-stride, -k : 0;   
    istart = (istart > start_row) ? istart : start_row;
    iend = (N-1 < N-1-k) ? N-1 : N-1-k;
    iend = (iend < end_row) ? iend : end_row;
    for(n = istart; n <= iend; n++){
      y[n] += (data[(size_t)i*stride+n-index] * x[n+k]);
    }
  }
}

