#include "conversions.h"

/*int create_blocks(int start, int end, int *array){
  int i = start;
  int num_blocks = 0;
  while(i < end - 1){
    start_block = i;
    num_blocks++;
    i++;
    while(i < end -1 && array[i] == array[i-1] + 1){
      i++;
    }
  }
  if(i == end - 1 && array[i] != array[i-1] + 1)
    num_blocks++;
  return num_blocks;
}*/

/*void csr_bcsr(int nz, int N, int *row_ptr, int *colind, MYTYPE *csr_val)
{
  for(i=0; i<N; i++){
    blocks_per_row[i] = create_blocks(row_ptr[i], row_ptr[i+1], colind);
  }

}
*/
void coo_csr(int nz, int N, int *row, int *col, MYTYPE *coo_val, int *row_ptr, int *colind, MYTYPE *val)
{
  int i, j, j0, r, c;
  MYTYPE data;
  clock_t start, stop;

  start = clock();
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
  for(i = 0; i < N; i++){
    sort(row_ptr[i],row_ptr[i+1], colind, val);
  }
  stop = clock();

//  printf("Elapsed time for coo to csr =\t %g milliseconds\n", ((double)(stop - start)) / CLOCKS_PER_SEC * 1000);
}

void csr_ell(int *row_ptr, int *colind, MYTYPE *val, int **indices, MYTYPE **data, int N, int* num_cols, int nnz)
{
  int i, j, k, col, max = 0, temp = 0;
  clock_t start, stop;

  start = clock();
  for(i = 0; i < N ; i++){
    temp = row_ptr[i+1] - row_ptr[i];
    if (max < temp)
      max = temp;
  }
  *num_cols = max;
 
  //printf("num of cols : %d\n", max);
  /*for(i = 0; i < N; i++){
    data[i] = (MYTYPE*)calloc(max, sizeof(MYTYPE));
    indices[i] = (int*)calloc(max, sizeof(int));
    k = 0;
    for(j = row_ptr[i]; j < row_ptr[i+1]; j++){
      data[i][k] = val[j];
      indices[i][k] = colind[j];
      k++;
    }
  }*/
    if(((size_t)N * max) > pow(2, 27) || (((size_t)N * max)/nnz) > 3){
      fprintf(stderr, "too large");
      exit(1);
    }
  *data = (MYTYPE*)malloc((size_t)N * max * sizeof(MYTYPE));
  if(*data == NULL){
    fprintf(stderr, "couldn't allocate ell_data using malloc");
    exit(1);
  }
  *indices = (int*)malloc((size_t)N * max * sizeof(int));
  if(*indices == NULL){
    fprintf(stderr, "couldn't allocate indices using malloc");
    exit(1);
  }
  for(i = 0; i < N; i++){
    k = 0;
    for(j = row_ptr[i]; j < row_ptr[i+1]; j++){
      (*data)[i*max+k] = val[j];
      (*indices)[i*max+k] = colind[j];
      k++;
    }
  }
  stop = clock();
 // printf("Elapsed time for csr to ell =\t %g milliseconds\n", ((double)(stop - start)) / CLOCKS_PER_SEC * 1000);

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
  *stride = N;
  size_t size = (size_t)num_diag * *stride;
  if(size > pow(2, 27) || (size/nnz) > 3){
    fprintf(stderr, "too large");
    exit(1);
  }

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
  //*stride = N - min;
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
          //if((*offset)[k] < 0)
            //move = N - *stride; 
          //(*data)[(size_t)k * *stride + i - move] = val[j];
          (*data)[(size_t)i * num_diag + k] = val[j];
          break;
        }
      }
    }
  } 
  stop = clock();
  //printf("Elapsed time for csr to dia =\t %g milliseconds\n", ((double)(stop - start)) / CLOCKS_PER_SEC * 1000);
}

void csr_diaii(int *row_ptr, int *colind, MYTYPE *val, int **offset, MYTYPE **data, int N, int *nd, int *stride, int nnz)
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
  *stride = N;
  size_t size = (size_t)num_diag * *stride;
  /*if(size > pow(2, 27) || (size/nnz) > 3){
    fprintf(stderr, "too large");
    exit(1);
  }*/

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
  //*stride = N - min;
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

void csr_ellii(int *row_ptr, int *colind, MYTYPE *val, int **indices, MYTYPE **data, int N, int* num_cols, int nnz)
{ 
  int i, j, k, col, max = 0, temp = 0;
  clock_t start, stop;
  
  start = clock(); 
  for(i = 0; i < N ; i++){
    temp = row_ptr[i+1] - row_ptr[i];
    if (max < temp)
      max = temp;
  }
  *num_cols = max;
  
  //printf("num of cols : %d\n", max);
  /*for(i = 0; i < N; i++){
    data[i] = (MYTYPE*)calloc(max, sizeof(MYTYPE));
    indices[i] = (int*)calloc(max, sizeof(int));
    k = 0;
    for(j = row_ptr[i]; j < row_ptr[i+1]; j++){
      data[i][k] = val[j];
      indices[i][k] = colind[j];
      k++;
    }
  }*/
    if(((size_t)N * max) > pow(2, 27) || (((size_t)N * max)/nnz) > 3){
      fprintf(stderr, "too large");
      exit(-1);
    }
  *data = (MYTYPE*)malloc((size_t)N * max * sizeof(MYTYPE));
  if(*data == NULL){
    fprintf(stderr, "couldn't allocate ell_data using malloc");
    exit(1);
  }
  *indices = (int*)malloc((size_t)N * max * sizeof(int));
  if(*indices == NULL){
    fprintf(stderr, "couldn't allocate indices using malloc");
    exit(1);
  }

  for(i = 0; i < N; i++){
    k = 0;
    for(j = row_ptr[i]; j < row_ptr[i+1]; j++){
      (*data)[k*N+i] = val[j];
      (*indices)[k*N+i] = colind[j];
      k++;
    }
  }

  stop = clock();
 // printf("Elapsed time for csr to ell =\t %g milliseconds\n", ((double)(stop - start)) / CLOCKS_PER_SEC * 1000);

}

void csr_custom(int *row_ptr, int *colind, MYTYPE *val, int **offset, MYTYPE **data, int N, int *nd, int **ptr)
{
  int i, j, num_diag, min, *ind, index, diag_no, col, k;
  clock_t start, stop;
  int move;
  num_diag = 0;
  size_t size = 0;
  int istart;
  int sum = 0;

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
  *ptr = (int*)malloc(num_diag * sizeof(int));
  if(*offset == NULL || *ptr == NULL){
    fprintf(stderr, "couldn't allocate *offset using malloc\n");
    exit(1);
  }

diag_no = -((2*N-1)/2);
  min = abs(diag_no);
  index = 0;
  for(i=0;i<2*N-1;i++){
    if(ind[i]){
      (*ptr)[index] = size;
      (*offset)[index++] = diag_no;
      size += N - abs(diag_no);
      if(min > abs(diag_no))
        min = abs(diag_no);
    }
    diag_no++;
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
        int o = (*offset)[k];
        if(col - i == o){
          istart = (0 < -o) ? -o : 0;
          move = i - istart;
          (*data)[(*ptr)[k] + move] = val[j];
          sum++;
          break;
        }
      }
    }
  }
  stop = clock();
  //printf("Elapsed time for csr to dia =\t %g milliseconds\n", ((double)(stop - start)) / CLOCKS_PER_SEC * 1000);
}



