#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include "mmio.h"
#include <math.h>
#include <libgen.h>
#include "spmv_coo.h"
#include "spmv_csr.h"
#include "spmv_dia.h"
#include "spmv_diaii.h"
#include "spmv_ell.h"
#include "spmv_ellii.h"
#include "conversions.h"
#include "utils.h"
#include "config.h"
#include "papi.h"

#define OUTER_MAX 30

int get_timings_spmv(char *filename,char *format, char * workers);

void *wasm_spmv_coo_wrapper(void *ptr){
  int inside;
  spmv_coo_struct *s = (spmv_coo_struct *) ptr;

  int nz = s->len;
  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_coo(s->row, s->col, s->val, nz, s->N, s->x, s->y);
  }
  pthread_exit(NULL);
}

void *wasm_spmv_csr_wrapper(void *ptr){
  int inside;
  spmv_csr_struct *s = (spmv_csr_struct *) ptr;

  int N = s->len;
  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_csr(s->rowptr, s->col, s->val, N, s->x, s->y);
  }
  pthread_exit(NULL);
}

void *wasm_spmv_dia_wrapper(void *ptr){
  int inside;
  spmv_dia_struct *s = (spmv_dia_struct *) ptr;

  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_dia(s->offset, s->data, s->start_row, s->end_row, s->nd, s->N, s->stride, s->x, s->y);
  }
  pthread_exit(NULL);
}


void *wasm_spmv_ell_wrapper(void *ptr){
  int inside;
  spmv_ell_struct *s = (spmv_ell_struct *) ptr;

  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_ell(s->indices, s->data, s->start_row, s->end_row, s->nc, s->N, s->x, s->y);
  }
  pthread_exit(NULL);
}

void *wasm_spmv_diaii_wrapper(void *ptr){
  int inside;
  spmv_diaii_struct *s = (spmv_diaii_struct *) ptr;

  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_diaii(s->offset, s->data, s->start_row, s->end_row, s->nd, s->N, s->stride, s->x, s->y);
  }
  pthread_exit(NULL);
}

void *wasm_spmv_ellii_wrapper(void *ptr){
  int inside;
  spmv_ellii_struct *s = (spmv_ellii_struct *) ptr;

  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_ellii(s->indices, s->data, s->start_row, s->end_row, s->nc, s->N, s->x, s->y);
  }
  pthread_exit(NULL);
}



void *spmv_coo_wrapper(void *ptr){
  int inside;
  spmv_coo_struct *s = (spmv_coo_struct *) ptr;
  cpu_set_t cpuset; 
  int cpu = s->tid;
  CPU_ZERO(&cpuset);       //clears the cpuset
  CPU_SET(cpu, &cpuset);
  sched_setaffinity(0, sizeof(cpuset), &cpuset);
  
  int nz = s->len;
  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_coo(s->row, s->col, s->val, nz, s->N, s->x, s->y);
  }
  pthread_exit(NULL);
}

void *spmv_csr_wrapper(void *ptr){
  int inside;
  spmv_csr_struct *s = (spmv_csr_struct *) ptr;
  /*cpu_set_t cpuset; 
  int cpu = s->tid;
  CPU_ZERO(&cpuset);       //clears the cpuset
  CPU_SET(cpu, &cpuset); */
  //sched_setaffinity(0, sizeof(cpuset), &cpuset);

  int N = s->len;
  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_csr(s->rowptr, s->col, s->val, N, s->x, s->y);
  }
  pthread_exit(NULL);
}

void *spmv_dia_wrapper(void *ptr){
  int inside;
  spmv_dia_struct *s = (spmv_dia_struct *) ptr;
  /*cpu_set_t cpuset; 
  int cpu = s->tid;
  CPU_ZERO(&cpuset);       //clears the cpuset
  CPU_SET(cpu, &cpuset); */
  //sched_setaffinity(0, sizeof(cpuset), &cpuset);

  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_dia(s->offset, s->data, s->start_row, s->end_row, s->nd, s->N, s->stride, s->x, s->y);
  }
  pthread_exit(NULL);
}

void *spmv_ell_wrapper(void *ptr){
  int inside;
  spmv_ell_struct *s = (spmv_ell_struct *) ptr;
  /*cpu_set_t cpuset; 
  int cpu = s->tid;
  CPU_ZERO(&cpuset);       //clears the cpuset
  CPU_SET(cpu, &cpuset); */
  //sched_setaffinity(0, sizeof(cpuset), &cpuset);

  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_ell(s->indices, s->data, s->start_row, s->end_row, s->nc, s->N, s->x, s->y);
  }
  pthread_exit(NULL);
}

void *spmv_diaii_wrapper(void *ptr){
  unsigned long int tid;
  int events[3] = {PAPI_L1_DCM, PAPI_L2_DCM, PAPI_L3_TCM}, ret, event_set=PAPI_NULL;
  long long values[3];
  if (PAPI_thread_init(pthread_self) != PAPI_OK)
    exit(1);
  if ((tid = PAPI_thread_id()) == (unsigned long int)-1)
    exit(1);
  PAPI_register_thread();
  ret = PAPI_create_eventset(&event_set);
  if(ret != PAPI_OK){
    fprintf(stderr, "create event set error\n");
    exit(1);
  }
  ret = PAPI_add_events(event_set, events, 3);
  if(ret != PAPI_OK){
    fprintf(stderr, "add event error\n");
    exit(1);
  }
  int inside;
  spmv_diaii_struct *s = (spmv_diaii_struct *) ptr;
  int cpu = s->tid;
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);       //clears the cpuset
  CPU_SET(cpu, &cpuset);
  sched_setaffinity(0, sizeof(cpuset), &cpuset);

  //printf("%u %d\n", current_thread, sched_getcpu());
  int inside_max = s->inside_max;
  if ((ret = PAPI_start(event_set)) != PAPI_OK) {
    fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(ret));
    exit(1);
  }
  for(inside = 0; inside < inside_max; inside++){
    spmv_diaii(s->offset, s->data, s->start_row, s->end_row, s->nd, s->N, s->stride, s->x, s->y);
  }
  if ((ret = PAPI_read(event_set, values)) != PAPI_OK) {
    fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
    exit(1);
  }
  PAPI_unregister_thread();
  pthread_exit(NULL);
}

void *spmv_ellii_wrapper(void *ptr){
  int inside;
  spmv_ellii_struct *s = (spmv_ellii_struct *) ptr;
  /*cpu_set_t cpuset; 
  int cpu = s->tid;
  CPU_ZERO(&cpuset);       //clears the cpuset
  CPU_SET(cpu, &cpuset);
  sched_setaffinity(0, sizeof(cpuset), &cpuset);*/

  int inside_max = s->inside_max;
  for(inside = 0; inside < inside_max; inside++){
    spmv_ellii(s->indices, s->data, s->start_row, s->end_row, s->nc, s->N, s->x, s->y);
  }
  pthread_exit(NULL);
}

int mainTrial(char *filename, char *format, char *workers) {
  int a =  get_timings_spmv(filename, format, workers);

  return a;
}

int main(int argc, char* argv[])
{
  struct timespec start_tp, stop_tp;
  clockid_t clk_id;
  clk_id = CLOCK_REALTIME;
  int ret_code, r, c, k;
  MM_typecode matcode;
  FILE *f, *g;
  int M, N, entries, anz, i, j, *row, *col, *row_ptr, *colind, *offset, *ptr, *indices, num_diags, stride, num_cols;
  MYTYPE *x, *y, *val, *dia_data, *ell_data, *coo_val;
  double v;
  clock_t start, stop;
  double sum = 0, mean = 0, sd = 0, variance = 0;
  int inside = 0, inside_max = 100000, outer_max = OUTER_MAX;
  double exec_arr[OUTER_MAX];
  int events[3] = {PAPI_L1_DCM, PAPI_L2_DCM, PAPI_L3_TCM}, ret, event_set=PAPI_NULL;
  long long values[3];

 /* int numberOfProcessors = sysconf(_SC_NPROCESSORS_ONLN);
  printf("Number of processors: %d\n", numberOfProcessors);
  pthread_attr_t attr;
  cpu_set_t cpus;
  pthread_attr_init(&attr);*/

  if(argc < 4){
    fprintf(stderr, "Usage: %s [martix-market-filename] format_num num_workers\n", argv[0]);
      exit(1);
  }
  else{
    if((f = fopen(argv[1], "r")) == NULL)
      exit(1);
  }

  /*if (PAPI_num_counters() < 3) {
     fprintf(stderr, "No hardware counters here, or PAPI not supported.\n");
     exit(1);
  }*/


  ret = PAPI_library_init(PAPI_VER_CURRENT);
  if (ret != PAPI_VER_CURRENT && ret > 0) {
    fprintf(stderr,"PAPI library version mismatch!\n");
    exit(1);
  }
  if (ret < 0) {
    fprintf(stderr, "Initialization error!\n");
    exit(1);
  }

  ret = PAPI_create_eventset(&event_set);
  if(ret != PAPI_OK){
    fprintf(stderr, "create event set error\n");
    exit(1);
  }
  ret = PAPI_add_events(event_set, events, 3);
  if(ret != PAPI_OK){
    fprintf(stderr, "add event error\n");
    exit(1);
  }

  int num_workers = atoi(argv[3]);  

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

  if(anz > 1000000) inside_max = 1;
  else if (anz > 100000) inside_max = 10;
  else if (anz > 50000) inside_max = 50;
  else if(anz > 10000) inside_max = 100;
  else if(anz > 2000) inside_max = 1000;
  else if(anz > 100) inside_max = 10000;
  inside_max *= 6;

  pthread_t threads[num_workers];
  quickSort(row, col, coo_val, 0, anz-1);

  if(atoi(argv[2]) == 1){
    spmv_coo_struct *s[num_workers];
    int nnz_per_worker = anz / num_workers;
    int rem = anz - nnz_per_worker * num_workers;
    int t_index = 0;
    int t = 0;
    /*for(i = 0; i < anz; i++){
      printf("%d %d %f\n", row[i], col[i], coo_val[i]); 
    }*/
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_coo_struct*)malloc(sizeof(spmv_coo_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nz = anz;
      s[t]->N = N;
      s[t]->row = row + t_index;
      //printf("start %d\n", s[t]->row[0]);
      s[t]->col =  col + t_index;
      s[t]->val = coo_val + t_index;
      s[t]->x = x;
      s[t]->y = (MYTYPE*) calloc(N, sizeof(MYTYPE));
      t_index += nnz_per_worker;
      s[t]->len = nnz_per_worker; 
      //printf("end %d\n", s[t]->row[nnz_per_worker-1]);
      s[t]->inside_max = inside_max;
    }
    s[t-1]->len += rem;
    //printf("\n%d\n", fletcher_sum_1d_array_int(row, anz));
    int iret;
    if ((ret = PAPI_start(event_set)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(ret));
      exit(1);
    }
    /*if ((ret = PAPI_start_counters(events, 3)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(ret));
      exit(1);
    }*/
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
        zero_arr(N, s[t]->y);
      //start = clock();
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, spmv_coo_wrapper, (void *)(s[t])); 
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
        for(int j = 0; j < N; j++){
          y[j] += s[t]->y[j];
        }
      }
      //stop = clock();
      //printf("%d\n", fletcher_sum(y, N)); 
      //exec_arr[i] = 1.0/1000000 * 2 * anz / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      //sum += (double)(stop - start);
      clock_gettime(clk_id, &stop_tp);
      //printf("%f\n",(double) ((stop_tp.tv_sec - start_tp.tv_sec) + ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    /*if ((ret = PAPI_read_counters(values, 3)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
      exit(1);
    }*/
    if ((ret = PAPI_read(event_set, values)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
      exit(1);
    }
    printf("%d,", num_workers);
    printf("%d,", outer_max);
    printf("%d,", inside_max);
    printf("%d,", N);
    printf("%d,", anz);
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    } 
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,", sd);
    printf("%g,", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    //printf("%d\n", fletcher_sum_1d_array_int(row, anz)); 
    //printf("%d\n", fletcher_sum_1d_array_int(col, anz)); 
    //printf("%d\n", fletcher_sum(coo_val, anz)); 
    //printf("%d\n", fletcher_sum(x, M)); 
    printf("%d\n", fletcher_sum(y, N)); 
    printf("L1 data cache misses is %lld\n", values[0]);
    printf("L2 data cache misses is %lld\n", values[1]);
    printf("L3 data cache misses is %lld\n", values[2]);
    /*for(i = 0; i < N; i++){
      printf("%.12lf\n", y[i]);
    }*/
  }
  else if(atoi(argv[2]) == 2){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    spmv_csr_struct *s[num_workers];
    int rows_per_worker = N / num_workers;
    int rem = N - rows_per_worker * num_workers;
    int t_index = 0;
    int t = 0;
    /*for(i = 0; i < N; i++){
      for(j = row_ptr[i]; j < row_ptr[i+1]; j++)
        printf("%d %d %f\n", i, colind[j], val[j]);
    }*/
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_csr_struct*)malloc(sizeof(spmv_csr_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nz = anz;
      s[t]->N = N;
      s[t]->rowptr = row_ptr + t_index;
      s[t]->col =  colind;
      s[t]->val = val;
      s[t]->x = x;
      s[t]->y = y + t_index;
      t_index += rows_per_worker;
      s[t]->len = rows_per_worker;
      s[t]->inside_max = inside_max;
    }
    s[t-1]->len += rem;
    int iret;
    if ((ret = PAPI_start(event_set)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(ret));
      exit(1);
    }
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
        //zero_arr(N, s[t]->y);
      //start = clock();
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, spmv_csr_wrapper, (void *)(s[t]));
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
        /*for(int j = 0; j < N; j++){
          y[j] += s[t]->y[j];
        }*/
      }
      //stop = clock();
      //printf("%d\n", fletcher_sum(y, N)); 
      //exec_arr[i] = 1.0/1000000 * 2 * anz / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      //sum += (double)(stop - start);
      clock_gettime(clk_id, &stop_tp);
      //printf("%lf\n",(double) ((stop_tp.tv_sec - start_tp.tv_sec) + ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    if ((ret = PAPI_read(event_set, values)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
      exit(1);
    }
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    }
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,", sd);
    printf("%g,", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    //printf("%d\n", fletcher_sum_1d_array_int(row, anz)); 
    //printf("%d\n", fletcher_sum_1d_array_int(col, anz)); 
    //printf("%d\n", fletcher_sum(coo_val, anz)); 
    //printf("%d\n", fletcher_sum(x, M)); 
    printf("%d\n", fletcher_sum(y, N));
    printf("L1 data cache misses is %lld\n", values[0]);
    printf("L2 data cache misses is %lld\n", values[1]);
    printf("L3 data cache misses is %lld\n", values[2]);
    /*for(i = 0; i < N; i++){
      printf("%.12lf\n", y[i]);
    }*/
  }
  else if(atoi(argv[2]) == 3){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    csr_dia(row_ptr, colind, val, &offset, &dia_data, N, &num_diags, &stride, anz);
    spmv_dia_struct *s[num_workers];
    int rows_per_worker = N / num_workers;
    int rem = N - rows_per_worker * num_workers;
    int t_index = 0;
    int t = 0;
    /*for(i = 0; i < N; i++){
      for(j = 0; j < num_diags; j++)
        printf("%f, ", dia_data[i*num_diags+j]);
      printf("\n");
    }
    printf("hi");
    for(j = 0; j < num_diags; j++)
      printf("%d, ", offset[j]);
    printf("\n");*/
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_dia_struct*)malloc(sizeof(spmv_dia_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nd = num_diags;
      s[t]->N = N;
      //printf("num diags %d\n", s[t]->nd);
      s[t]->stride = stride;
      //printf("stride %d\n", s[t]->stride);
      s[t]->start_row = t_index/num_diags;
      //printf("start row %d\n", s[t]->start_row);
      s[t]->data = dia_data;
      s[t]->offset = offset;
      s[t]->x = x;
      s[t]->y = y;
      t_index += rows_per_worker * num_diags;
      s[t]->end_row = s[t]->start_row + rows_per_worker;
      //printf("end row %d\n", s[t]->end_row);
      s[t]->inside_max = inside_max;
    }
    s[t-1]->end_row += rem;
    int iret;
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
        //zero_arr(N, s[t]->y);
      //start = clock();
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, spmv_dia_wrapper, (void *)(s[t]));
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
        /*for(int j = 0; j < N; j++){
          y[j] += s[t]->y[j];
        }*/
      }
      //stop = clock();
      //printf("%d\n", fletcher_sum(y, N)); 
      //exec_arr[i] = 1.0/1000000 * 2 * anz / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      //sum += (double)(stop - start);
      clock_gettime(clk_id, &stop_tp);
      //printf("%f\n",(double) ((stop_tp.tv_sec - start_tp.tv_sec) + ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    }
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,", sd);
    printf("%g,", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    //printf("%d\n", fletcher_sum_1d_array_int(row, anz)); 
    //printf("%d\n", fletcher_sum_1d_array_int(col, anz)); 
    //printf("%d\n", fletcher_sum(coo_val, anz)); 
    //printf("%d\n", fletcher_sum(x, M)); 
    printf("%d\n", fletcher_sum(y, N));
    /*for(i = 0; i < N; i++){
      printf("%f\n", y[i]);
    }*/
  }
  else if(atoi(argv[2]) == 4){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    csr_ell(row_ptr, colind, val, &indices, &ell_data, N, &num_cols, anz);

    spmv_ell_struct *s[num_workers];
    int rows_per_worker = N / num_workers;
    int rem = N - rows_per_worker * num_workers;
    int t_index = 0;
    int t;
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_ell_struct*)malloc(sizeof(spmv_ell_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nc = num_cols;
      s[t]->N = N;
      s[t]->start_row = t_index/num_cols;
      s[t]->data = ell_data;
      s[t]->indices = indices;
      s[t]->x = x;
      s[t]->y = y;
      t_index += rows_per_worker * num_cols;
      s[t]->end_row = s[t]->start_row + rows_per_worker;
      s[t]->inside_max = inside_max;
    }
    s[t-1]->end_row += rem;
    int iret;
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, spmv_ell_wrapper, (void *)(s[t]));
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
      }
      clock_gettime(clk_id, &stop_tp);
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    }
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,", sd);
    printf("%g,", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    printf("%d\n", fletcher_sum(y, N));
  }
  
  else if(atoi(argv[2]) == 5){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    csr_diaii(row_ptr, colind, val, &offset, &dia_data, N, &num_diags, &stride, anz);
    spmv_diaii_struct *s[num_workers];
    int rows_per_worker = N / num_workers;
    int rem = N - rows_per_worker * num_workers;
    int t_index = 0;
    int t = 0;
    pthread_attr_t attr;
    int ret;
    int scope;
    /* initialize an attribute to the default value */
    ret = pthread_attr_init(&attr);
    if (ret != 0){
      fprintf(stderr, "pthread attr init error\n");
      exit(1);
    } 
    ret = pthread_attr_getscope(&attr, &scope);
    if (ret != 0){
      fprintf(stderr, "pthread attr get scope\n");
      exit(1);
    } 
    if(scope == PTHREAD_SCOPE_PROCESS)
      printf("process scope\n");
    else if(scope == PTHREAD_SCOPE_SYSTEM)
      printf("system scope\n");
    /*for(i = 0; i < N; i++){
      for(j = 0; j < num_diags; j++)
        printf("%f, ", dia_data[i*num_diags+j]);
      printf("\n");
    }
    for(j = 0; j < num_diags; j++)
      printf("%d, ", offset[j]);
    printf("\n");*/
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_diaii_struct*)malloc(sizeof(spmv_diaii_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nd = num_diags;
      s[t]->N = N;
      //printf("num diags %d\n", s[t]->nd);
      s[t]->stride = stride;
      //printf("stride %d\n", s[t]->stride);
      s[t]->start_row = t_index/num_diags;
      //printf("start row %d\n", s[t]->start_row);
      s[t]->data = dia_data;
      s[t]->offset = offset;
      s[t]->x = x;
      //s[t]->y = (MYTYPE*) calloc(N, sizeof(MYTYPE));
      s[t]->y = y; 
      t_index += rows_per_worker * num_diags;
      s[t]->end_row = s[t]->start_row + rows_per_worker - 1;
      //printf("end row %d\n", s[t]->end_row);
      s[t]->inside_max = inside_max;
    }
    s[t-1]->end_row += rem;
    int iret;
    /*if ((ret = PAPI_start_counters(events, 3)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(ret));
      exit(1);
    }*/
    if ((ret = PAPI_start(event_set)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(ret));
      exit(1);
    }
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
        //zero_arr(N, s[t]->y);
      //start = clock();
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, spmv_diaii_wrapper, (void *)(s[t]));
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
        /*for(int j = 0; j < N; j++){
          y[j] += s[t]->y[j];
        }*/
      }
      //stop = clock();
      //printf("%d\n", fletcher_sum(y, N)); 
      //exec_arr[i] = 1.0/1000000 * 2 * anz / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      //sum += (double)(stop - start);
      clock_gettime(clk_id, &stop_tp);
      //printf("%f\n",(double) ((stop_tp.tv_sec - start_tp.tv_sec) + ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    /*if ((ret = PAPI_read_counters(values, 3)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
      exit(1);
    }*/
    if ((ret = PAPI_read(event_set, values)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
      exit(1);
    }
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    }
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,", sd);
    printf("%g,", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    //printf("%d\n", fletcher_sum_1d_array_int(row, anz)); 
    //printf("%d\n", fletcher_sum_1d_array_int(col, anz)); 
    //printf("%d\n", fletcher_sum(coo_val, anz)); 
    //printf("%d\n", fletcher_sum(x, M)); 
    printf("%d\n", fletcher_sum(y, N));
    /*for(i = 0; i < N; i++){
      printf("%f\n", y[i]);
    }*/
    printf("L1 data cache misses is %lld\n", values[0]);
    printf("L2 data cache misses is %lld\n", values[1]);
    printf("L3 data cache misses is %lld\n", values[2]);
  }

  else if(atoi(argv[2]) == 6){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    csr_ellii(row_ptr, colind, val, &indices, &ell_data, N, &num_cols, anz);

    spmv_ellii_struct *s[num_workers];
    int rows_per_worker = N / num_workers;
    int rem = N - rows_per_worker * num_workers;
    int t_index = 0;
    int t;
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_ellii_struct*)malloc(sizeof(spmv_ellii_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nc = num_cols;
      s[t]->N = N;
      s[t]->start_row = t_index/num_cols;
      s[t]->data = ell_data;
      s[t]->indices = indices;
      s[t]->x = x;
      s[t]->y = y;
      t_index += rows_per_worker * num_cols;
      s[t]->end_row = s[t]->start_row + rows_per_worker;
      s[t]->inside_max = inside_max;
    }
    s[t-1]->end_row += rem;
    int iret;
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, spmv_ellii_wrapper, (void *)(s[t]));
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
      }
      clock_gettime(clk_id, &stop_tp);
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    }
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,", sd);
    printf("%g,", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    printf("%d\n", fletcher_sum(y, N));
  }

  
  if (f !=stdin) 
    fclose(f);
  pthread_exit(NULL);

  free(x);
  free(y);
  free(row);
  free(col);
  free(coo_val);

  return 0;
}

int get_timings_spmv(char *filename, char* format, char* workers)
{
  int num_workers = atoi(workers);
  struct timespec start_tp, stop_tp;
  clockid_t clk_id;
  clk_id = CLOCK_REALTIME;
  int ret_code, r, c, k;
  MM_typecode matcode;
  FILE *f, *g;
  int M, N, entries, anz, i, j, *row, *col, *row_ptr, *colind, *offset, *ptr, *indices, num_diags, stride, num_cols;
  MYTYPE *x, *y, *val, *dia_data, *ell_data, *coo_val;
  double v;
  clock_t start, stop;
  double sum = 0, mean = 0, sd, variance = 0;
  int inside = 0, inside_max = 100000, outer_max = OUTER_MAX;
  double exec_arr[OUTER_MAX];
  pthread_attr_t attr;
  cpu_set_t cpus;
  pthread_attr_init(&attr);

  if((f = fopen(filename, "r")) == NULL)
    exit(1);

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

  if(anz > 1000000) inside_max = 1;
  else if (anz > 100000) inside_max = 10;
  else if (anz > 50000) inside_max = 50;
  else if(anz > 10000) inside_max = 100;
  else if(anz > 2000) inside_max = 1000;
  else if(anz > 100) inside_max = 10000;
  inside_max *= 6;
 

  pthread_t threads[num_workers];
  quickSort(row, col, coo_val, 0, anz-1);

  if(atoi(format) == 1){
    spmv_coo_struct *s[num_workers];
    int nnz_per_worker = anz / num_workers;
    int rem = anz - nnz_per_worker * num_workers;
    int t_index = 0;
    int t = 0;
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_coo_struct*)malloc(sizeof(spmv_coo_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nz = anz;
      s[t]->N = N;
      s[t]->row = row + t_index;
      s[t]->col =  col + t_index;
      s[t]->val = coo_val + t_index;
      s[t]->x = x;
      s[t]->y = (MYTYPE*) calloc(N, sizeof(MYTYPE));
      t_index += nnz_per_worker;
      s[t]->len = nnz_per_worker; 
      s[t]->inside_max = inside_max;
    }
    s[t-1]->len += rem;
    int iret;
    for (i=0; i < outer_max; i++){
      //printf("%d\n", i);
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
        zero_arr(N, s[t]->y);
      //start = clock();
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, wasm_spmv_coo_wrapper, (void *)(s[t])); 
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
        for(int j = 0; j < N; j++){
          y[j] += s[t]->y[j];
        }
      }
      //stop = clock();
      //printf("%d\n", fletcher_sum(y, N)); 
      //exec_arr[i] = 1.0/1000000 * 2 * anz / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      //sum += (double)(stop - start);
      clock_gettime(clk_id, &stop_tp);
      //printf("%f\n",(double) ((stop_tp.tv_sec - start_tp.tv_sec) + ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    printf("%d,\n", num_workers);
    printf("%d,\n", outer_max);
    printf("%d,\n", inside_max);
    printf("%d,\n", N);
    printf("%d,\n", anz);
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    } 
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,\n", sd);
    printf("%g,\n", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    //printf("%d\n", fletcher_sum_1d_array_int(row, anz)); 
    //printf("%d\n", fletcher_sum_1d_array_int(col, anz)); 
    //printf("%d\n", fletcher_sum(coo_val, anz)); 
    //printf("%d\n", fletcher_sum(x, M)); 
    printf("%d\n", fletcher_sum(y, N)); 
  }
  
  else if(atoi(format) == 2){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    spmv_csr_struct *s[num_workers];
    int rows_per_worker = N / num_workers;
    int rem = N - rows_per_worker * num_workers;
    int t_index = 0;
    int t = 0;
    /*for(i = 0; i < N; i++){
      for(j = row_ptr[i]; j < row_ptr[i+1]; j++)
        printf("%d %d %f\n", i, colind[j], val[j]);
    }*/
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_csr_struct*)malloc(sizeof(spmv_csr_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nz = anz;
      s[t]->N = N;
      s[t]->rowptr = row_ptr + t_index;
      s[t]->col =  colind;
      s[t]->val = val;
      s[t]->x = x;
      s[t]->y = y + t_index;
      t_index += rows_per_worker;
      s[t]->len = rows_per_worker;
      s[t]->inside_max = inside_max;
    }
    s[t-1]->len += rem;
    int iret;
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
        //zero_arr(N, s[t]->y);
      //start = clock();
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, wasm_spmv_csr_wrapper, (void *)(s[t]));
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
        /*for(int j = 0; j < N; j++){
          y[j] += s[t]->y[j];
        }*/
      }
      //stop = clock();
      //printf("%d\n", fletcher_sum(y, N)); 
      //exec_arr[i] = 1.0/1000000 * 2 * anz / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      //sum += (double)(stop - start);
      clock_gettime(clk_id, &stop_tp);
      //printf("%f\n",(double) ((stop_tp.tv_sec - start_tp.tv_sec) + ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    }
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,\n", sd);
    printf("%g,\n", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    //printf("%d\n", fletcher_sum_1d_array_int(row, anz)); 
    //printf("%d\n", fletcher_sum_1d_array_int(col, anz)); 
    //printf("%d\n", fletcher_sum(coo_val, anz)); 
    //printf("%d\n", fletcher_sum(x, M)); 
    printf("%d\n", fletcher_sum(y, N));
    /*for(i = 0; i < N; i++){
      printf("%f\n", y[i]);
    }*/
  }

  else if(atoi(format) == 3){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    csr_dia(row_ptr, colind, val, &offset, &dia_data, N, &num_diags, &stride, anz);
    spmv_dia_struct *s[num_workers];
    int rows_per_worker = N / num_workers;
    int rem = N - rows_per_worker * num_workers;
    int t_index = 0;
    int t = 0;
    /*for(i = 0; i < N; i++){
      for(j = 0; j < num_diags; j++)
        printf("%f, ", dia_data[i*num_diags+j]);
      printf("\n");
    }
    for(j = 0; j < num_diags; j++)
      printf("%d, ", offset[j]);
    printf("\n");*/
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_dia_struct*)malloc(sizeof(spmv_dia_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nd = num_diags;
      s[t]->N = N;
      //printf("num diags %d\n", s[t]->nd);
      s[t]->stride = stride;
      //printf("stride %d\n", s[t]->stride);
      s[t]->start_row = t_index/num_diags;
      //printf("start row %d\n", s[t]->start_row);
      s[t]->data = dia_data;
      s[t]->offset = offset;
      s[t]->x = x;
      s[t]->y = y;
      t_index += rows_per_worker * num_diags;
      s[t]->end_row = s[t]->start_row + rows_per_worker;
      //printf("end row %d\n", s[t]->end_row);
      s[t]->inside_max = inside_max;
    }
    s[t-1]->end_row += rem;
    int iret;
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
        //zero_arr(N, s[t]->y);
      //start = clock();
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, wasm_spmv_dia_wrapper, (void *)(s[t]));
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
        /*for(int j = 0; j < N; j++){
          y[j] += s[t]->y[j];
        }*/
      }
      //stop = clock();
      //printf("%d\n", fletcher_sum(y, N)); 
      //exec_arr[i] = 1.0/1000000 * 2 * anz / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      //sum += (double)(stop - start);
      clock_gettime(clk_id, &stop_tp);
      //printf("%f\n",(double) ((stop_tp.tv_sec - start_tp.tv_sec) + ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    }
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,\n", sd);
    printf("%g,\n", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    //printf("%d\n", fletcher_sum_1d_array_int(row, anz)); 
    //printf("%d\n", fletcher_sum_1d_array_int(col, anz)); 
    //printf("%d\n", fletcher_sum(coo_val, anz)); 
    //printf("%d\n", fletcher_sum(x, M)); 
    printf("%d\n", fletcher_sum(y, N));
     /*for(i = 0; i < N; i++){
      printf("%f\n", y[i]);
    }*/
  }
  else if(atoi(format) == 4){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    csr_ell(row_ptr, colind, val, &indices, &ell_data, N, &num_cols, anz);

    spmv_ell_struct *s[num_workers];
    int rows_per_worker = N / num_workers;
    int rem = N - rows_per_worker * num_workers;
    int t_index = 0;
    int t;
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_ell_struct*)malloc(sizeof(spmv_ell_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nc = num_cols;
      s[t]->N = N;
      s[t]->start_row = t_index/num_cols;
      s[t]->data = ell_data;
      s[t]->indices = indices;
      s[t]->x = x;
      s[t]->y = y;
      t_index += rows_per_worker * num_cols;
      s[t]->end_row = s[t]->start_row + rows_per_worker;
      s[t]->inside_max = inside_max;
    }
    s[t-1]->end_row += rem;
    int iret;
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, wasm_spmv_ell_wrapper, (void *)(s[t]));
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
      }
      clock_gettime(clk_id, &stop_tp);
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    }
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,\n", sd);
    printf("%g,\n", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    printf("%d\n", fletcher_sum(y, N));
  }

  else if(atoi(format) == 5){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    csr_diaii(row_ptr, colind, val, &offset, &dia_data, N, &num_diags, &stride, anz);
    spmv_diaii_struct *s[num_workers];
    int rows_per_worker = N / num_workers;
    int rem = N - rows_per_worker * num_workers;
    int t_index = 0;
    int t = 0;
    /*for(i = 0; i < N; i++){
      for(j = 0; j < num_diags; j++)
        printf("%f, ", dia_data[i*num_diags+j]);
      printf("\n");
    }
    for(j = 0; j < num_diags; j++)
      printf("%d, ", offset[j]);
    printf("\n");*/
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_diaii_struct*)malloc(sizeof(spmv_diaii_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nd = num_diags;
      s[t]->N = N;
      //printf("num diags %d\n", s[t]->nd);
      s[t]->stride = stride;
      //printf("stride %d\n", s[t]->stride);
      s[t]->start_row = t_index/num_diags;
      //printf("start row %d\n", s[t]->start_row);
      s[t]->data = dia_data;
      s[t]->offset = offset;
      s[t]->x = x;
      s[t]->y = y;
      //s[t]->y = (MYTYPE*) calloc(N, sizeof(MYTYPE));
      t_index += rows_per_worker * num_diags;
      s[t]->end_row = s[t]->start_row + rows_per_worker - 1;
      //printf("end row %d\n", s[t]->end_row);
      s[t]->inside_max = inside_max;
    }
    s[t-1]->end_row += rem;
    int iret;
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
        zero_arr(N, s[t]->y);
      //start = clock();
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, wasm_spmv_diaii_wrapper, (void *)(s[t]));
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
        /*for(int j = 0; j < N; j++){
          y[j] += s[t]->y[j];
        }*/
      }
      //stop = clock();
      //printf("%d\n", fletcher_sum(y, N)); 
      //exec_arr[i] = 1.0/1000000 * 2 * anz / ((double)(stop - start)/ CLOCKS_PER_SEC); 
      //sum += (double)(stop - start);
      clock_gettime(clk_id, &stop_tp);
      //printf("%f\n",(double) ((stop_tp.tv_sec - start_tp.tv_sec) + ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0))); 
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    }
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,\n", sd);
    printf("%g,\n", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    //printf("%d\n", fletcher_sum_1d_array_int(row, anz)); 
    //printf("%d\n", fletcher_sum_1d_array_int(col, anz)); 
    //printf("%d\n", fletcher_sum(coo_val, anz)); 
    //printf("%d\n", fletcher_sum(x, M)); 
    printf("%d\n", fletcher_sum(y, N));
    /*for(i = 0; i < N; i++){
      printf("%f\n", y[i]);
    }*/
  }

  else if(atoi(format) == 6){
    coo_csr(anz, N, row, col, coo_val, row_ptr, colind, val);
    csr_ellii(row_ptr, colind, val, &indices, &ell_data, N, &num_cols, anz);

    spmv_ellii_struct *s[num_workers];
    int rows_per_worker = N / num_workers;
    int rem = N - rows_per_worker * num_workers;
    int t_index = 0;
    int t;
    for(t = 0; t < num_workers; t++){
      s[t] = (spmv_ellii_struct*)malloc(sizeof(spmv_ellii_struct));
      if(s[t] == NULL){
        printf("struct couldn't be allocated\n");
        exit(1);
      }
      s[t]->tid = t;
      s[t]->nc = num_cols;
      s[t]->N = N;
      s[t]->start_row = t_index/num_cols;
      s[t]->data = ell_data;
      s[t]->indices = indices;
      s[t]->x = x;
      s[t]->y = y;
      t_index += rows_per_worker * num_cols;
      s[t]->end_row = s[t]->start_row + rows_per_worker;
      s[t]->inside_max = inside_max;
    }
    s[t-1]->end_row += rem;
    int iret;
    for (i=0; i<outer_max; i++){
      zero_arr(N, y);
      for(t = 0; t < num_workers; t++)
      clock_gettime(clk_id, &start_tp);
      for(t = 0; t < num_workers; t++){
        iret = pthread_create(&threads[t], NULL, wasm_spmv_ellii_wrapper, (void *)(s[t]));
        if(iret != 0){
          printf("Error in pthreads!\n");
          exit(1);
        }
      }
      for(t = 0; t < num_workers; t++){
        pthread_join(threads[t], NULL);
      }
      clock_gettime(clk_id, &stop_tp);
      exec_arr[i] = 1.0/1000000 * 2 * anz * inside_max/ ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
      sum += ((double)((stop_tp.tv_sec - start_tp.tv_sec)+ ((stop_tp.tv_nsec - start_tp.tv_nsec) / 1000000000.0)));
    }
    mean  = 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum));
    for (i=0; i<outer_max; i++){
      variance += (mean - exec_arr[i]) * (mean - exec_arr[i]);
    }
    variance /= outer_max;
    sd = sqrt(variance);
    printf("%f,\n", sd);
    printf("%g,\n", 1.0/1000000 * 2 * anz * outer_max * inside_max/ ((double)(sum)));
    printf("%d\n", fletcher_sum(y, N));
  }


  if (f !=stdin) 
    fclose(f);

  free(x);
  free(y);
  free(row);
  free(row_ptr);
  free(col);
  free(coo_val);
  free(colind);
  free(val);

  return 0;
}
