# fait-maison-spmv
Sparse matrix-vector multiplication (SpMV) implementations for each of the four formats -- COO, CSR, DIA and ELL.

    y = Ax
where _A_ is a _M x N_ sparse matrix with _nnz_ number of non-zeros, _x_ is a dense input vector of size _N_ and _y_ is a dense output vector of size _M_.

## Sequential 
For C, JavaScript, and WebAssembly via Emscripten. These implementations follow closely to conventional implementations of SpMV that target cache-based superscalar uniprocessor machines.

## Parallel
For C using pthreads and WebAssembly via Emscripten.

## Input Matrices
The input matrix is required to be in Matrix Market format (.mtx). Some real-life examples of sparse matrices in this external format can be obtained from The SuiteSparse Matrix Collection (formerly the University of Florida Sparse Matrix Collection) at https://sparse.tamu.edu

Our implementations currently support square sparse matrices, where M == N. We categorize the matrices based on the size equal to sizeof(x) + sizeof(y)

    1. X-Large : don't fit in the L3 cache.
    2. Large : fit in L3 cache, but don't fit in the L2 cache.
    3. Medium : fit in L2 cache, but don't fit in the L1 cache.
    4. Small : fit in the L1 cache.
    
For our cheetah machine, L1 : 32KB, L2 : 256KB, L3 : 12MB
Therefore, for single-precision storage, 

     X-Large : N > 1,572,864
     Large : 32,768 < N < 1,572,864
     Medium : 4,096 < N < 32,768
     Small : N < 4,096

## Matrix Features

    make features
    ./run_features <matrix_market_input_file_path>

1. Matrix dimension, _N_
2. Number of non-zeros, _nnz_ 
3. Density, _nnz/(N * N)_
4. Number of non-zeros per row, *nnz_row* (min, max, mean, sd)
5. Column width per row, *col_width_row* (min, max, mean, sd)
6. Scatter ratio per row, *scatter_row* (min, max, mean, sd)
7. Miss density per row, *miss_density_row*

### Flop:Byte

_Flop:Byte_ is the ratio of total number of floating point operations to the total number of bytes of the storage format.
For example, for single-precision CSR format sparse matrix ->
    
    Flop : Byte = (2 * nnz) / (8 * nnz + 12 * N)
    
Following are the scatter plots for our benchmark suite (correlation coefficient : 0.607) based on different sizes 
(comparing the SpMV performance in MFLOPS vs flop:byte) -> 
    
 <img src="./tests/results/scatter_plots/xlarge_flop_byte_scatter_plot.png" width = 420/>   <img src="./tests/results/scatter_plots/large_flop_byte_scatter_plot.png" width = 420/>
 
 <img src="./tests/results/scatter_plots/medium_flop_byte_scatter_plot.png" width = 420/>   <img src="./tests/results/scatter_plots/small_flop_byte_scatter_plot.png" width = 420/>
    
The matrices with high _Flop:Byte_ ratio seem to perform better than others. The low performance of matrices with high _Flop:Byte_ ratio can be explained by low _nnz_row_, which is equal to the number of times inner loop of SpMV CSR executes. Therefore, the overhead of the inner loop seems to bring performance down. Also, the huge variation between the number of non-zeros per row potentially leads to high branch mispredictions.  

### Density

    Density = nnz/(N * N)
 
Following are the scatter plots for our benchmark suite based on different sizes 
(comparing the SpMV performance in MFLOPS vs Density) -> 

<img src="./tests/results/scatter_plots/xlarge_density_scatter_plot.png" width = 420/>   <img src="./tests/results/scatter_plots/large_density_scatter_plot.png" width = 420/>
 
 <img src="./tests/results/scatter_plots/medium_density_scatter_plot.png" width = 420/>   <img src="./tests/results/scatter_plots/small_density_scatter_plot.png" width = 420/>
 
### Cache Miss Density

   Theoretically, low miss density leads to better performance. It means correlation coefficient should be closer to -1.
   Following are the scatter plots for our benchmark suite (correlation coefficient : -0.372) based on different sizes 
(comparing the SpMV performance in MFLOPS vs Miss Density) -> 

<img src="./tests/results/scatter_plots/xlarge_miss_density_scatter_plot.png" width = 420/>   <img src="./tests/results/scatter_plots/large_miss_density_scatter_plot.png" width = 420/>
 
 <img src="./tests/results/scatter_plots/medium_miss_density_scatter_plot.png" width = 420/>   <img src="./tests/results/scatter_plots/small_miss_density_scatter_plot.png" width = 420/>

## Experiments

Please follow [ManLang18-SpMV](https://github.com/Sable/manlang18-spmv) for the experimental data and scripts.

## Run C implementation

  ### Single precision

    make float
    ./run_float <matrix_market_input_file_path> <format_string>
  
  ### Double precision

    make double
    ./run_double <matrix_market_input_file_path> <format_string>
    
where <format_string> can be COO, CSR, DIA and ELL.
  
  ### Debug with PAPI
    make float DEBUG=1
    ./run_float <matrix_market_input_file_path> <format_string>

## Run JavaScript implementation
  
  ### single-precision
  
    ./run.py -b <browser> -p single <matrix_market_input_file_path>
  
  ### double-precision
  
    ./run.py -b <browser> -p double <matrix_market_input_file_path>
    
where \<browser\> is chrome for Google Chrome and firefox for Mozilla Firefox


## Run WebAssembly implementation

  ### single-precision

    ./index.js -b <browser> -p single -f <matrix_market_input_file_path>

  ### double-precision

    ./index.js -b <browser> -p double -f <matrix_market_input_file_path>

where \<browser\> is chrome for Google Chrome and firefox for Mozilla Firefox


## Feedback

Please contact [Prabhjot](mailto:prabhjot.sandhu@mail.mcgill.ca).
