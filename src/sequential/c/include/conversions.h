#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include "utils.h"
#include "spmv_coo.h"
#include "config.h"

void coo_csr(int nz, int N, int *row, int *col, MYTYPE *coo_val, int *row_ptr, int *colind, MYTYPE *csr_val);
void csr_dia(int *row_ptr, int *colind, MYTYPE *val, int **offset, MYTYPE **data, int N, int* nd, int *stride, int nnz);
void csr_ell(int *row_ptr, int *colind, MYTYPE *val, int **indices, MYTYPE **data, int N, int* nc, int nnz);
