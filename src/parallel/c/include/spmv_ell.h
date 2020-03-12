#include<stdio.h>
#include<math.h>
#include<time.h>
#include "utils.h"
#include "config.h"

typedef struct spmv_ell_struct{
  int tid;
  int start_row, end_row;
  int nc, N;
  int inner_max;
  int *indices;
  MYTYPE *data, *x, *y;
}spmv_ell_struct;

void spmv_ell(int *indices, MYTYPE *data, int start_row, int end_row, int nc, int N, MYTYPE *x, MYTYPE *y);
