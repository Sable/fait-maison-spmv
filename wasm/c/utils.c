#include<stdio.h>
#include<math.h>
#include "config.h"


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
