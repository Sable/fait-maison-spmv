#include<stdio.h>
#include<math.h>
#include<time.h>
#include "utils.h"
#include "config.h"

void spmv_csr(int *row_ptr, int *colind, MYTYPE *val, int N, MYTYPE *x, MYTYPE *y);
