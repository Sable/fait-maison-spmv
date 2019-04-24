#include<stdio.h>
#include<stdlib.h>
#include "mmio.h"
#include "config.h"
#include "utils.h"
#include<math.h>

long long sum_i(int *arr, int n)
{
  int i;
  long long sum = 0;
  for(i = 0; i < n; i++)
    sum += arr[i]; 
  return sum;
}

float mean_i(int *arr, int n)
{
  long long sum = sum_i(arr, n);
  return sum/(float)n;
}

double sum_f(float *arr, int n)
{
  int i;
  double sum = 0.0;
  for(i = 0; i < n; i++)
    sum += arr[i]; 
  return sum;
}

float geo_mean(float *arr, int n){
  float sum = 0;
  int i;

  for(i = 0; i < n; i++)
    sum += log(arr[i]);

  sum /= n;

  return exp(sum);
}

float mean_f(float *arr, int n)
{
  double sum = sum_f(arr, n);
  return sum/n;
}

float sd_i(int *arr, int n, float mean_i)
{
  float var = 0.0;
  int i;
  for(i = 0; i < n; i++){
    var += (mean_i - arr[i]) * (mean_i - arr[i]);
  }
  var = var/n;
  return sqrt(var);
}

float sd_f(float *arr, int n, float mean_i)
{
  float var = 0.0;
  int i;
  for(i = 0; i < n; i++){
    var += (mean_i - arr[i]) * (mean_i - arr[i]);
  }
  var = var/n;
  return sqrt(var);
}

int main(int argc, char* argv[])
{ 
  MM_typecode matcode;
  int i, k, M, N, entries, anz, r, c, ret_code;
  double v;
  int *row, *col, *col_width, *first_col_row, *last_col_row, last_col = -1;
  int *x_used;
  float *nnz_row, *scatter_row, *misses;
  int total_misses;
  int min_nnz_row, max_nnz_row, min_col_width, max_col_width;
  MYTYPE *coo_val;
  FILE *f;

  if(argc < 2){
    fprintf(stderr, "Usage: %s [martix-market-filename] \n", argv[0]);
      exit(1);
  }
  else{
    if((f = fopen(argv[1], "r")) == NULL){
      fprintf(stderr, "couldn't open file\n");
      exit(1);
    }
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
  nnz_row = (float*)calloc(N, sizeof(float));
  if(nnz_row == NULL){
    fprintf(stderr, "couldn't allocate nnz_row using malloc");
    exit(1);
  }
  col_width = (int*)calloc(N, sizeof(int));
  if(col_width == NULL){
    fprintf(stderr, "couldn't allocate col_width using malloc");
    exit(1);
  }
  first_col_row = (int*)calloc(N, sizeof(int));
  if(first_col_row == NULL){
    fprintf(stderr, "couldn't allocate first col row using malloc");
    exit(1);
  }
  last_col_row = (int*)calloc(N, sizeof(int));
  if(last_col_row == NULL){
    fprintf(stderr, "couldn't allocate last col row using malloc");
    exit(1);
  }
  scatter_row = (float*)calloc(N, sizeof(float));
  if(scatter_row == NULL){
    fprintf(stderr, "couldn't allocate scatter_row using malloc");
    exit(1);
  }
  misses = (float*)calloc(N, sizeof(float));
  if(misses == NULL){
    fprintf(stderr, "couldn't allocate misses using malloc");
    exit(1);
  }
  x_used = (int*)calloc(N, sizeof(int));
  if(x_used == NULL){
    fprintf(stderr, "couldn't allocate x used using malloc");
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


  min_nnz_row = N;
  max_nnz_row = 0;
  min_col_width = N;
  max_col_width = 0;

  for(i = 0; i < N; i++){
    first_col_row[i] = N;
    last_col_row[i] = -1;
  }
 
  for(i = 0; i < anz; i++){
    x_used[col[i]]++;
    if(last_col >= 0 && row[i-1] < row[i])
      last_col = -1;
    nnz_row[row[i]]++;
    if(first_col_row[row[i]] > col[i]) 
      first_col_row[row[i]] = col[i];
    if(last_col_row[row[i]] < col[i]) 
      last_col_row[row[i]] = col[i];
    if(last_col >= 0 && (col[i] - 16) < last_col)
      continue;
    else{
      misses[row[i]]++;
      last_col = col[i];
    }
  }
  
  total_misses = 0;
  for(i = 0; i < N; i++){
    if(nnz_row[i] > max_nnz_row)
      max_nnz_row = nnz_row[i];
    if(nnz_row[i] < min_nnz_row)
      min_nnz_row = nnz_row[i];
    col_width[i] = last_col_row[i] - first_col_row[i] + 1;
    if(col_width[i] < 0)
      col_width[i] = 0;
    if(col_width[i] > max_col_width)
      max_col_width = col_width[i];
    if(col_width[i] < min_col_width)
      min_col_width = col_width[i]; 
    if(col_width[i] > 0)
      scatter_row[i] = ((float)nnz_row[i])/col_width[i];
    if(nnz_row[i] > 0)
      misses[i] = misses[i]/nnz_row[i];
    if(x_used[i] > 0)
      total_misses++;
    nnz_row[i] = (float)nnz_row[i]/N;
    if(nnz_row[i] == 0)
      nnz_row[i] = 0.1/N;
    if(misses[i] == 0)
      misses[i] = 0.1/N;
    if(scatter_row[i] == 0)
      scatter_row[i] = 0.1/N;
  }

  printf("%d,%d,", N, anz);
  printf("%e,", anz/((float)N * N));
  printf("%e,%e,%e,", (float)min_nnz_row/N, (float)max_nnz_row/N, geo_mean(nnz_row, N));
  printf("%e,%e,%e,", (float)min_col_width/N, (float)max_col_width/N, mean_i(col_width, N));
  printf("%.3f,", geo_mean(scatter_row, N)); 
  printf("%e,", geo_mean(misses, N));
  printf("%e\n", (float)(2 * anz)/(8 * anz + 12 * N));

  free(row);
  free(col);
  free(coo_val);
  free(nnz_row);
  free(first_col_row);
  free(last_col_row);
  free(col_width);
  free(scatter_row);
  free(misses);
  free(x_used);

  return 0;
}

