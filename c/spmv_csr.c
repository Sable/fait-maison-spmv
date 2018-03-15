#include "spmv_csr.h"
#include "config.h"


void spmv_csr(int *row_ptr, int *colind, MYTYPE *val, int N, MYTYPE *x, MYTYPE *y)
{
  int i, j;
  MYTYPE temp;
  for(i = 0; i < N ; i++)
  {
    temp = y[i];
    for(j = row_ptr[i]; j < row_ptr[i+1]; j++){
      temp += val[j] * x[colind[j]];
    }
    y[i] = temp;
  }
}
