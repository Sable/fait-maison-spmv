#include<stdio.h>
#include<math.h>
#include<time.h>
#include "utils.h"
#include "config.h"

typedef struct spmv_csr_struct{
  int tid;
  int nz, N;
  int len;
  int inside_max;
  int *rowptr, *col;
  MYTYPE *val, *x, *y;
}spmv_csr_struct;

void spmv_csr(int *row_ptr, int *colind, MYTYPE *val, int N, MYTYPE *x, MYTYPE *y);
