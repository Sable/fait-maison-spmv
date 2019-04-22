#include "spmv_coo.h"
#include "config.h"


void spmv_coo(int *rowind, int *colind, MYTYPE *val, int nz, int N, MYTYPE *x, MYTYPE *y)
{
  int i;
  for(i = 0; i < nz ; i++)
    y[rowind[i]] += val[i] * x[colind[i]];
}

