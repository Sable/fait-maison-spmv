#ifndef SPMV_COO_H
#define SPMV_COO_H

#include<stdio.h>
#include<math.h>
#include<time.h>
#include "utils.h"
#include "config.h"

void spmv_coo(int *row, int *col, MYTYPE *coo_val, int nz, int N, MYTYPE *x, MYTYPE *y);
#endif
