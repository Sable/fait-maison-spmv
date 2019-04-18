#include "utils.h"

int string_compare(char *s1, char *s2)
{
  int i;
  if(strlen(s1) != strlen(s2))
    return -1;
  for(i = 0; i < strlen(s1); i++){
    if(tolower(s1[i]) != tolower(s2[i]))
      return -1;
  } 
  return 0;
}

int count_nnz(FILE *f)
{
  MM_typecode matcode;
  int i, M, N, entries, anz, r, c, ret_code;
  double v;

  if(mm_read_banner(f, &matcode) != 0){
    fprintf(stderr, "Could not process Matrix Market banner.\n");
    exit(1);
  }
  /*  This is how one can screen matrix types if their application */
  /*  only supports a subset of the Matrix Market data types.      */

  if(mm_is_complex(matcode) && mm_is_matrix(matcode) && mm_is_sparse(matcode)){
    fprintf(stderr, "Sorry, this application does not support ");
    fprintf(stderr, "Market Market type: [%s]\n", mm_typecode_to_str(matcode));
    exit(1);
  }

  /* find out size of sparse matrix .... */
  if((ret_code = mm_read_mtx_crd_size(f, &M, &N, &entries)) !=0)
    exit(1);

  /* reseve memory for matrices */

  if(mm_is_symmetric(matcode)){
    anz = 0;
    if(!mm_is_pattern(matcode)){
      for (i=0; i<entries; i++)
      {
        fscanf(f, "%d %d %lf\n", &r, &c, &v);
        if( v < 0 || v > 0){
          if(r == c)
            anz++;
          else
            anz = anz + 2;
        }
      }
    }
    else{
      for (i=0; i<entries; i++)
      {
        fscanf(f, "%d %d\n", &r, &c);
        if(r == c)
          anz++;
        else
          anz = anz + 2;
      }
    }
  }
  else{
      anz = 0;
      for (i=0; i<entries; i++)
      {
        fscanf(f, "%d %d %lf\n", &r, &c, &v);
        if( v < 0 || v > 0)
          anz++;
      }
  }
  return anz;
}


void swap(int* a, int* b)
{ 
  int t = *a;
  *a = *b;
  *b = t;
}
void swap_val(MYTYPE* a, MYTYPE* b)
{
  MYTYPE t = *a;
  *a = *b;
  *b = t;
}


void quickSort(int arr[], int arr2[], MYTYPE arr3[], int left, int right)
{
  int i = left, j = right;
  int pivot = arr[(left + right) / 2];
  int pivot_col = arr2[(left + right) / 2];

  /* partition */
  while(i <= j) {
    while(arr[i] < pivot || (arr[i] == pivot && arr2[i] < pivot_col))
      i++;
    while(arr[j] > pivot || (arr[j] == pivot && arr2[j] > pivot_col))
      j--;
    if(i <= j) {
      swap(&arr[i], &arr[j]);
      swap(&arr2[i], &arr2[j]);
      swap_val(&arr3[i], &arr3[j]);
      i++;
      j--;
    }
  }

  /* recursion */
  if(left < j)
    quickSort(arr, arr2, arr3, left, j);
  if (i < right)
    quickSort(arr, arr2, arr3, i, right);
}


void init_arr(int N, MYTYPE* a)
{	
	int i;
	for (i=0; i<N;i++) {
          a[i] = i;
	}
}
void zero_arr(int N, MYTYPE* a)
{	
	int i;
	for (i=0; i<N;i++) {
          a[i] = 0.0;
	}
}

//print array to std out
void print_arr(int N, char * name, double* array)
{	
	int i,j;	
	printf("\n%s\n",name);
	for (i=0;i<N;i++){
		for (j=0;j<N;j++) {
			printf("%g\t",array[N*i+j]);
		}
		printf("\n");
	}
}

void sort(int start, int end, int *array1, MYTYPE *array2)
{ 
  int i, j, temp;
  for(i = 0; i < end-start-1; i++){
    for(j = start; j < end-i-1; j++){
      if(array1[j] > array1[j+1]){
        temp = array1[j];
        array1[j] = array1[j+1];
        array1[j+1] = temp;
        temp = array2[j];
        array2[j] = array2[j+1];
        array2[j+1] = temp;
      }
    }
  }
}

void cmp(MYTYPE *y, MYTYPE *y1, int N)
{
  int i;
  for(i = 0; i < N; i++){
    if(y[i] != y1[i])
      fprintf(stderr, "values don't match at index %d", i);
  }
}

double matlab_modulo(double x, double y) {
    double n = floor(x/y);
    return (x - n*y);
}

int fletcher_sum(MYTYPE *a, int size) {
    double sum1 = 0;
    double sum2 = 0;
    for (int i = 0; i < size; ++i) {
        sum1 = matlab_modulo((sum1 + a[i]),255);
        sum2 = matlab_modulo((sum2 + sum1),255);
    }

    return sum2 * 256 + sum1;
}
int fletcher_sum_1d_array_int(int *a, int size) {
    double sum1 = 0;
    double sum2 = 0;
    for (int i = 0; i < size; ++i) {
        sum1 = matlab_modulo((sum1 + a[i]),255);
        sum2 = matlab_modulo((sum2 + sum1),255);
    }

    return sum2 * 256 + sum1;
}
