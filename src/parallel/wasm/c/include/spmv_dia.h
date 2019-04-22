#include<stdio.h>
#include<math.h>
#include<time.h>
#include "utils.h"
#include "config.h"

typedef struct spmv_dia_struct{
  int tid;
  int start_row, end_row;
  int nd, stride, N;
  int len;
  int inside_max;
  int *offset;
  MYTYPE *data, *x, *y;
}spmv_dia_struct;

void spmv_dia(int *offset, MYTYPE *data, int start_row, int end_row, int nd, int N, int stride, MYTYPE *x, MYTYPE *y);
void spmv_custom(int *offset, MYTYPE *data, int N, int nd, int *ptr, MYTYPE *x, MYTYPE *y);
