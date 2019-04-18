#include "spmv_dia.h"
#include "config.h"

void spmv_dia(int *offset, MYTYPE *data, int start_row, int end_row, int nd, int N, int stride, MYTYPE *x, MYTYPE *y)
{
  int i, row;
  int col;
  for(row = start_row; row < end_row; row++){ 
    //printf("row %d\n ", row); 
    for(i = 0; i < nd; i++){
      col = offset[i] + row;
      //printf("\tcol %d\n", col); 
      if(col >=0 && col < N){
        //printf("%d\n", (size_t)row*nd+i);
        y[row] += (data[(size_t)row*nd+i] * x[col]);
        //printf("\ty[%d] %f\n", row, y[row]); 
      }
    }
    //printf("\n");
  }
}

void spmv_custom(int *offset, MYTYPE *data, int N, int nd, int *ptr, MYTYPE *x, MYTYPE *y)
{
  int i, k, n, istart, iend, index;

  index = 0;
  for(i = 0; i < nd; i++){
    k = offset[i];
    istart = (0 < -k) ? -k : 0;
    iend = (N-1 < N-1-k) ? N-1 : N-1-k;
    for(n = istart; n <= iend; n++){
      y[n] += (data[index++] * x[n+k]);
    }
  }
}

