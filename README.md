# fait-maison-spmv
Sparse matrix-vector multiplication implementations for C and two web languages : JavaScript and WebAssembly for each of the four formats -- COO, CSR, DIA and ELL. 
Our implementations follow closely to conventional implementations of SpMV that target cache-based superscalar uniprocessor machines.
To have a fair comparison with JavaScript and WebAssembly which are single threaded, we have focused on uniprocessor sequential SpMV.

## Input Matrices
The input matrix is required to be in Matrix Market format (.mtx). Some real-life examples of sparse matrices in this external format 
can be obtained from The SuiteSparse Matrix Collection (formerly the University of Florida Sparse Matrix Collection) at https://sparse.tamu.edu

## Run C implementation

  ### single-precision

    make TEST="-D FLOAT"
    ./run <matrix_market_input_file_path> <format_index>
  
  ### double-precision

    make
    ./run <matrix_market_input_file_path> <format_index>
    
where <format_index> is 1 for COO, 2 for CSR, 3 for DIA and 4 for ELL

## Run JavaScript implementation
  
  ### single-precision
  
    ./run.py -b <browser> -p single <matrix_market_input_file_path>
  
  ### double-precision
  
    ./run.py -b <browser> -p double <matrix_market_input_file_path>
    
where \<browser\> is chrome for Google Chrome and firefox for Mozilla Firefox
  

## Run JavaScript implementation

  ### single-precision

    ./index.js -b <browser> -p single -f <matrix_market_input_file_path>

  ### double-precision

    ./index.js -b <browser> -p double -f <matrix_market_input_file_path>

where \<browser\> is chrome for Google Chrome and firefox for Mozilla Firefox
