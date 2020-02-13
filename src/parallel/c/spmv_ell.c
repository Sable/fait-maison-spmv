#include "spmv_ell.h"
#include "config.h"

void spmv_ell(int *indices, MYTYPE *data, int start_row, int end_row, int nc, int N, MYTYPE *x, MYTYPE *y)
{
  int i, row;
  int col;
  for(row = start_row; row < end_row; row++){
    for(i = 0; i < nc; i++){
      col = indices[(size_t)row*nc+i];
      if(col < 0)
        break;
      y[row] += (data[(size_t)row*nc+i] * x[col]);
    }
  }
}

