#include "conversions.h"

void coo_csr(int nz, int N, int *row, int *col, MYTYPE *coo_val, int *row_ptr, int *colind, MYTYPE *val)
{
  int i, j, j0, r, c;
  MYTYPE data;

  for (i=0; i<nz; i++)
    row_ptr[row[i]]++;

  j = 0;
  for (i=0; i<N; i++){
    j0 = row_ptr[i];
    row_ptr[i] = j;
    j += j0;
  }

  for (i=0; i<nz; i++){
    r = row[i];
    c = col[i];
    data = coo_val[i];
    j = row_ptr[r];
    colind[j] = c;
    val[j] = data;
    row_ptr[r]++;
  }

  for( i = N-1; i > 0; i--){
    row_ptr[i] = row_ptr[i-1];
  }
  row_ptr[0] = 0;
  row_ptr[N] = nz;
  /*for(i = 0; i < N; i++){
    sort(row_ptr[i],row_ptr[i+1], colind, val);
  }*/
  int *last_used = (int*)malloc(N * sizeof(int));
  if(last_used == NULL){
    fprintf(stderr, "couldn't allocate last_used using malloc");
    exit(1);
  } 
  for(i = 0; i < N; i++)
    last_used[i] = -1;
  int *reuse_distance = (int*)calloc(N, sizeof(int));
  if(reuse_distance == NULL){
    fprintf(stderr, "couldn't allocate reuse_distance using malloc");
    exit(1);
  }
  int *elems = (int*)calloc(N, sizeof(int));
  if(elems == NULL){
    fprintf(stderr, "couldn't allocate elems using calloc");
    exit(1);
  }
  
  int distance;
  for(i = 0; i < N; i++){
    elems[row_ptr[i+1] - row_ptr[i]]++; 
    for(j = row_ptr[i]; j < row_ptr[i+1]; j++){
      if(last_used[(colind[j])/16] == -1){
        last_used[(colind[j])/16] = i;
        continue;
      }
      distance = i - last_used[(colind[j])/16];
      if(distance <= i)
        reuse_distance[distance]++;
      last_used[(colind[j])/16] = i;
    }
  }
  int max_dis = reuse_distance[0];
  int max_i = 0;
  for(i = 0; i < N; i++){
    if(max_dis < reuse_distance[i]){
      max_dis = reuse_distance[i];
      max_i = i;
    }
  }
  int sum = 0;
  for(i = 16; i < N; i++){
    sum += reuse_distance[i];
  }
/*  printf("%d %d\n", max_i, max_dis);
  printf("reuse,percentage\n");
  for(i = 0; i < 16; i++){
    printf("%d,%d,%f\n", i, reuse_distance[i], (float)reuse_distance[i]*100/nz);
  }
  printf("\n");
  for(i = 0; i < 16; i++)
    printf("%d,%d,%f\n", i, elems[i], (float)elems[i]*100/N);
*/

//  printf("Elapsed time for coo to csr =\t %g milliseconds\n", ((double)(stop - start)) / CLOCKS_PER_SEC * 1000);
}

void csr_ell(int *row_ptr, int *colind, MYTYPE *val, int **indices, MYTYPE **data, int N, int* num_cols, int nnz)
{
  int i, j, k, col, max = 0, temp = 0;
  clock_t start, stop;

  for(i = 0; i < N ; i++){
    temp = row_ptr[i+1] - row_ptr[i];
    if (max < temp)
      max = temp;
  }
  *num_cols = max;
 
  if(((size_t)N * max) > pow(2, 27) || ((size_t)N * max * 2)/nnz > 10){
    fprintf(stderr, "too large");
    exit(1);
  }

  *data = (MYTYPE*)calloc((size_t)N * max, sizeof(MYTYPE));
  if(*data == NULL){
    fprintf(stderr, "couldn't allocate ell_data using calloc");
    exit(1);
  }
  *indices = (int*)malloc((size_t)N * max * sizeof(int));
  if(*indices == NULL){
    fprintf(stderr, "couldn't allocate indices using malloc");
    exit(1);
  }
  for(i = 0; i < max; i++){
    for(j = 0; j < N; j++){
      (*indices)[i*N+j] = -1;
    }
  }
  for(i = 0; i < N; i++){
    k = 0;
    for(j = row_ptr[i]; j < row_ptr[i+1]; j++){
      (*data)[k*N+i] = val[j];
      (*indices)[k*N+i] = colind[j];
      k++;
    }
  }
  
  int *last_used = (int*)malloc(N * sizeof(int));
  if(last_used == NULL){
    fprintf(stderr, "couldn't allocate x used using malloc");
    exit(1);
  }
  for(i = 0; i < N; i++)
    last_used[i] = -1;
  int *reuse_distance = (int*)calloc(nnz, sizeof(int));
  if(reuse_distance == NULL){
    fprintf(stderr, "couldn't allocate x used using malloc");
    exit(1);
  }
  
  int distance;
  int *h = (int *)calloc(N, sizeof(int));
  int count = 0;
  for(i = 0; i < max; i++){
    for(j = 0; j < N; j++){
      if((*indices)[i*N+j] == -1)
        continue;
      if(last_used[((*indices)[i*N+j])/16] == -1){
        count++;
        last_used[((*indices)[i*N+j])/16] = i*N+j;
        continue;
      }
      //memset(h, 0, sizeof(int)*N);
      //distance = count_uniq_elems((i*N+j), last_used[((*indices)[i*N+j])/16], (*indices), h);
      distance = (i*N+j) - last_used[((*indices)[i*N+j])/16] - 1;
      if(distance <= (i*N+j))
        reuse_distance[distance]++;
      last_used[((*indices)[i*N+j])/16] = i*N+j;
    }
  }
  int max_dis = reuse_distance[0];
  int max_i = 0;
  for(i = 0; i < nnz; i++){
    if(max_dis < reuse_distance[i]){
      max_dis = reuse_distance[i];
      max_i = i;
    }
  }
  int sum = 0;
  for(i = 0; i < 16; i++){
    sum += reuse_distance[i];
  }

  #ifdef ELLR
  printf("%f\n", ((float)sum*100)/nnz);
  exit(0);
  #endif
  /*printf("%d\n", count);
  printf("%d %d\n", max_i, max_dis);
  printf("reuse,percentage\n");
  for(i = 0; i < 16; i++){
    printf("%d,%d,%f\n", i, reuse_distance[i], (float)reuse_distance[i]*100/nnz);
  }
  printf("16+,%d\n", sum);
  */
  // Instead of storing 0 for the column index if the entry is missing, store the column index closer to it
  int prev_index = 0;
  for(i = 0; i < max; i++){
    for(j = 0; j < N; j++){
      if((*indices)[i*N+j] == -1)
        (*indices)[i*N+j] = prev_index;
      prev_index = (*indices)[i*N+j];
    }
  }
  //free(h);
}

void csr_dia(int *row_ptr, int *colind, MYTYPE *val, int **offset, MYTYPE **data, int N, int *nd, int *stride, int nnz)
{
  int i, j, num_diag, min, *ind, index, diag_no, col, k;
  clock_t start, stop;
  int move;
  num_diag = 0;

  ind = (int*)calloc(2*N-1, sizeof(int));
  if(ind == NULL){
    fprintf(stderr, "couldn't allocate ind using calloc\n");
    exit(1);
  }

  start = clock();
  for(i = 0; i < N ; i++){
    for(j = row_ptr[i]; j<row_ptr[i+1]; j++){
      if(!ind[N+colind[j]-i-1]++)
        num_diag++;
    }
  }
  *nd = num_diag;

  *offset = (int*)malloc(num_diag * sizeof(int));
  if(*offset == NULL){
    fprintf(stderr, "couldn't allocate *offset using malloc\n");
    exit(1);
  }
  diag_no = -((2*N-1)/2);
  min = abs(diag_no);
  index = 0;
  for(i=0;i<2*N-1;i++){
    if(ind[i]){
      (*offset)[index++] = diag_no;
      if(min > abs(diag_no))
        min = abs(diag_no); 
    }
    diag_no++;
  }
  *stride = N - min;
  size_t size = (size_t)num_diag * *stride;
  if(size > pow(2, 27) || (float)size/nnz > 10){
    fprintf(stderr, "too large");
    exit(1);
  }

  *data = (MYTYPE*)calloc(size, sizeof(MYTYPE));
  if(*data == NULL){
    fprintf(stderr, "couldn't allocate *data using calloc\n");
    fprintf(stderr, "%s\n", strerror(errno));
    fprintf(stderr, "%d\n", errno);
    exit(1);
  }
  for(i = 0; i < N; i++){
    for(j = row_ptr[i]; j < row_ptr[i+1]; j++){
      col = colind[j];
      for(k = 0; k < num_diag; k++){
        move = 0;
        if(col - i == (*offset)[k]){
          if((*offset)[k] < 0)
            move = N - *stride; 
          (*data)[(size_t)k * *stride + i - move] = val[j];
          break;
        }
      }
    }
  } 
  stop = clock();
  //printf("Elapsed time for csr to dia =\t %g milliseconds\n", ((double)(stop - start)) / CLOCKS_PER_SEC * 1000);
}
