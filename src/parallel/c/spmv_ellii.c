#include "spmv_ellii.h"
#include "config.h"

void spmv_ellii(int *indices, MYTYPE *data, int start_row, int end_row, int nc, int N, MYTYPE *x, MYTYPE *y)
{
  int i, row;
  int col;
  for(i = 0; i < nc; i++){
    for(row = start_row; row < end_row; row++){
      col = indices[(size_t)i*N+row];
      if(col >= 0)
        y[row] += data[(size_t)i*N+row] * x[indices[(size_t)i*N+row]];
    }
  }
}

