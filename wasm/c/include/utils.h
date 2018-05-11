#ifndef UTILS_H
#define UTILS_H

#include<stdio.h>
#include "config.h"

void init_arr(int N, MYTYPE* a);
void zero_arr(int N, MYTYPE* a);
void cmp(MYTYPE *y, MYTYPE *y1, int N);
void print_arr(int N, char * name, double* array);
void sort(int start, int end, int *array1, MYTYPE *array2);
int fletcher_sum(MYTYPE *a, int size);
int fletcher_sum_1d_array_int(int *a, int size);
#endif
