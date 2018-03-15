#include<stdio.h>
#include<math.h>
#include<time.h>
#include "utils.h"
#include "config.h"

void spmv_dia(int *offset, MYTYPE *data, int N, int nd, int stride, MYTYPE *x, MYTYPE *y);
