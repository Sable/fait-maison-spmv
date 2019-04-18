#ifndef UTILS_H
#define UTILS_H

#include<stdio.h>
#include <ctype.h> 
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include "config.h"
#include "mmio.h"

int count_nnz(FILE *f);
void quickSort(int arr[], int arr2[], MYTYPE arr3[], int low, int high);
int string_compare(char *, char *);
void init_arr(int N, MYTYPE* a);
void zero_arr(int N, MYTYPE* a);
void cmp(MYTYPE *y, MYTYPE *y1, int N);
void print_arr(int N, char * name, double* array);
void sort(int start, int end, int *array1, MYTYPE *array2);
int fletcher_sum(MYTYPE *a, int size);
int fletcher_sum_1d_array_int(int *a, int size);
#endif
