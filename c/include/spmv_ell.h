#include<stdio.h>
#include<math.h>
#include<time.h>
#include "utils.h"
#include "config.h"

void spmv_ell(int *indices, MYTYPE *data, int N, int nd, MYTYPE *x, MYTYPE *y);
