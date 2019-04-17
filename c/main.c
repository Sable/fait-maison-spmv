#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "mmio.h"
#include <math.h>
#include <libgen.h>
#include "spmv_csr.h"
#include "spmv_coo.h"
#include "spmv_dia.h"
#include "spmv_ell.h"
#include "conversions.h"
#include "utils.h"
#include "config.h"

#define OUTER_MAX 30

int main(int argc, char* argv[])
{
  int ret_code, r, c, k;
  MM_typecode matcode;
  FILE *f, *g;
  int M, N, entries, anz, i, j, *row, *col, *row_ptr, *colind, *offset, *indices, num_diags, stride, num_cols;
  MYTYPE *x, *y, *val, *dia_data, *ell_data, *coo_val;
  double v;
  clock_t start, stop;
  double exec_arr[OUTER_MAX];
  double sum = 0, mean = 0, sd, variance = 0;
  int inner, inner_max;

  if(argc < 2){
    fprintf(stderr, "Usage: %s [martix-market-filename]\n", argv[0]);
      exit(1);
  }
  else{
    if((f = fopen(argv[1], "r")) == NULL)
      exit(1);
  }

  anz = count_nnz(f);

  rewind(f);
  if (mm_read_banner(f, &matcode) != 0)
  {
    fprintf(stderr, "Could not process Matrix Market banner.\n");
    exit(1);
  }
  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &entries)) !=0)
      exit(1);
  
  if(M > N) 
    N = M;
  else
    M = N;

  row = (int*)malloc(anz * sizeof(int));
  if(row == NULL){
    fprintf(stderr, "couldn't allocate row using malloc");
    exit(1);
  }
  col = (int*)malloc(anz * sizeof(int));
  if(col == NULL){
    fprintf(stderr, "couldn't allocate col using malloc");
    exit(1);
  }
  coo_val = (MYTYPE*)malloc(anz * sizeof(MYTYPE));
  if(coo_val == NULL){
    fprintf(stderr, "couldn't allocate val using malloc");
    exit(1);
  }
  row_ptr = (int*)calloc(N+1, sizeof(int));
  if(row_ptr == NULL){
    fprintf(stderr, "couldn't allocate row_ptr using malloc");
    exit(1);
  }
  colind = (int*)malloc(anz * sizeof(int));
  if(colind == NULL){
    fprintf(stderr, "couldn't allocate colind using malloc");
    exit(1);
  }
  val = (MYTYPE*)malloc(anz * sizeof(MYTYPE));
  if(val == NULL){
    fprintf(stderr, "couldn't allocate val using malloc");
    exit(1);
  }
  x=(MYTYPE*) malloc( sizeof(MYTYPE)*N );
  if(x == NULL){
    fprintf(stderr, "couldn't allocate x using malloc");
    exit(1);
  }
  init_arr(N,x);
  y=(MYTYPE*) calloc(N, sizeof(MYTYPE));
  if(y == NULL){
    fprintf(stderr, "couldn't allocate y using calloc");
    exit(1);
  }

  /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
  /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
  /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

  k = 0;
  if(mm_is_symmetric(matcode)){
    if(!mm_is_pattern(matcode)){
      for (i=0; i<entries; i++){
        fscanf(f, "%d %d %lf\n", &r, &c, &v);
        if( v < 0 || v > 0){
          row[k] = r - 1;
          col[k]= c - 1;
          coo_val[k] = v;
          if(r == c){
            k++;
          }
          else{
            row[k+1] = col[k]; 
            col[k+1] = row[k];
            coo_val[k+1] = v;
            k = k + 2;
          }
        }
      }
    }
    else{
      for (i=0; i<entries; i++){
        fscanf(f, "%d %d\n", &r, &c);
        row[k] = r - 1;
        col[k]= c - 1;
        coo_val[k] = 1.0;
        if(r == c){
          k++;
        }
        else{
          row[k+1] = col[k]; 
          col[k+1] = row[k];
          coo_val[k+1] = 1.0;
          k = k + 2;
        }
      }
    }
  }
  else {
    if(!mm_is_pattern(matcode)){
      for (i=0; i<entries; i++){
        fscanf(f, "%d %d %lf\n", &r, &c, &v);
        if( v < 0 || v > 0){
          row[k] = r - 1;
          col[k]= c - 1;
          coo_val[k] = v;
          k++;
        }
      }
    }
    else {
      for (i=0; i<entries; i++){
        fscanf(f, "%d %d\n", &r, &c);
        row[i] = r - 1;
        col[i]= c - 1;
        coo_val[i] = 1.0;
      }
    }
  }

  quickSort(row, col, coo_val, 0, anz-1);


  if(anz > 1000000) inner_max = 1;
  else if (anz > 100000) inner_max = 10;
  else if (anz > 50000) inner_max = 50;
  else if(anz > 10000) inner_max = 100;
  else if(anz > 2000) inner_max = 1000;
  else if(anz > 100) inner_max = 10000;

  if(!string_compare(argv[2], "coo")){
    for (i=0; i<OUTER_MAX; i++){
      zero_arr(N, y);
      start = clock();
      for(inner = 0; inner < inner_max; inner++)
        spmv_coo(row, col, coo_val, anz, N, x, y);
      stop = clock();
      exec_arr[i] = 1.0/1000000 * 2 * anz * inner_max/ ((double)(stop - start)/ CLOCKS_PER_SEC); 
      sum += (double)(stop - start);
    }
    printf("%d,", N);
    printf("%d,", anz);
    mean  = 1.0/1000000 * 2 * anz * inner_max * OUTER_MAX / ((double)(sum) / CLOCKS_PER_SEC);
    for (i=0; i<OUTER_MAX; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    } 
    variance /= OUTER_MAX;
    sd = sqrt(variance);
    printf("%f,", sd);
    printf("%g,", mean);
    printf("%d", fletcher_sum(y, M)); 
  }

  if(!string_compare(argv[2], "csr")){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    for (i=0; i<OUTER_MAX; i++){
      zero_arr(N, y);
      start = clock();
      for(inner = 0; inner < inner_max; inner++)
        spmv_csr(row_ptr, colind, val, N, x, y);
      stop = clock();
      exec_arr[i] = 1.0/1000000 * 2 * anz * inner_max / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      sum += (double)(stop - start);
    }
    mean  = 1.0/1000000 * 2 * anz * inner_max * OUTER_MAX / ((double)(sum) / CLOCKS_PER_SEC);
    for (i=0; i<OUTER_MAX; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    } 
    variance /= OUTER_MAX;
    sd = sqrt(variance);
    printf("%f,", sd);
    printf("%g,", mean);
    printf("%d", fletcher_sum(y, M)); 
  }
  if(!string_compare(argv[2], "dia")){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    csr_dia(row_ptr, colind, val, &offset, &dia_data, N, &num_diags, &stride);
    for (i=0; i<OUTER_MAX; i++){
      zero_arr(N, y);
      start = clock();
      for(inner = 0; inner < inner_max; inner++)
        spmv_dia(offset, dia_data, N, num_diags, stride, x, y);
      stop = clock();
      exec_arr[i] = 1.0/1000000 * 2 * anz * inner_max / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      sum += (double)(stop - start);
    }
    mean  = 1.0/1000000 * 2 * anz * inner_max * OUTER_MAX / ((double)(sum) / CLOCKS_PER_SEC);
    for (i=0; i<OUTER_MAX; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    } 
    variance /= OUTER_MAX;
    sd = sqrt(variance);
    printf("%f,", sd);
    printf("%g,", mean);
    printf("%d", fletcher_sum(y, M)); 
    free(dia_data);
    free(offset);
  }
  if(!string_compare(argv[2], "ell")){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    csr_ell(row_ptr, colind, val, &indices, &ell_data, N, &num_cols);
    for (i=0; i<OUTER_MAX; i++){
      zero_arr(N, y);
      start = clock();
      for(inner = 0; inner < inner_max; inner++)
        spmv_ell(indices, ell_data, N, num_cols, x, y);
      stop = clock();
      exec_arr[i] = 1.0/1000000 * 2 * anz * inner_max / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      sum += (double)(stop - start);
    }
    mean  = 1.0/1000000 * 2 * anz * inner_max * OUTER_MAX / ((double)(sum) / CLOCKS_PER_SEC);
    for (i=0; i<OUTER_MAX; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    } 
    variance /= OUTER_MAX;
    sd = sqrt(variance);
    printf("%f,", sd);
    printf("%g,", mean);
    printf("%d", fletcher_sum(y, M)); 
    free(ell_data);
    free(indices);
  }

  
  if (f !=stdin) 
    fclose(f);

  free(x);
  free(y);
  free(row_ptr);
  free(row);
  free(col);
  free(coo_val);
  free(colind);
  free(val);

  return 0;
}

