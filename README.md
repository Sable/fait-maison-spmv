# fait-maison-spmv
Sparse matrix-vector multiplication (SpMV) uniprocessor implementations for C and two web languages : JavaScript and WebAssembly for each of the four formats -- COO, CSR, DIA and ELL. These implementations follow closely to conventional implementations of SpMV that target cache-based superscalar uniprocessor machines.

## Input Matrices
The input matrix is required to be in Matrix Market format (.mtx). Some real-life examples of sparse matrices in this external format can be obtained from The SuiteSparse Matrix Collection (formerly the University of Florida Sparse Matrix Collection) at https://sparse.tamu.edu

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

## Experiments

Please follow [ManLang18-SpMV](https://github.com/Sable/manlang18-spmv) for the experimental data and scripts.

## Run C implementation

  ### single-precision

    make float
    ./run_float <matrix_market_input_file_path> <format_string>
  
  ### double-precision

    make double
    ./run_double <matrix_market_input_file_path> <format_string>
    
where <format_string> can be COO, CSR, DIA and ELL.

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
