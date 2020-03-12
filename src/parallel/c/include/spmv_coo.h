#ifndef SPMV_COO_H
#define SPMV_COO_H

#include<stdio.h>
#include<math.h>
#include<time.h>
#include "utils.h"
#include "config.h"

typedef struct spmv_coo_struct{
  int tid;
  int nz, N;
  int len;
  int inner_max;
  int *row, *col;
  MYTYPE *val, *x, *y;
}spmv_coo_struct;

void spmv_coo(int *rowind, int *colind, MYTYPE *val, int nz, int N, MYTYPE *x, MYTYPE *y);
#endif
