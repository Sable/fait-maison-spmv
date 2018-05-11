//var ndarray = require('ndarray')
//module.exports = {myFunc: spmv};

function matlab_modulo(x, y) {
  var n = Math.floor(x/y);
  return x - n*y;
}

function fletcher_sum(A) {
  var sum1 = 0;
  var sum2 = 0;

  for (var i = 0; i < A.length; ++i) {
    sum1 = matlab_modulo((sum1 + A[i]),255);
    sum2 = matlab_modulo((sum2 + sum1),255);
  }
  return sum2 * 256 + sum1;
}
function spmv_ell(data, indices, N, nc, x, y){
  var i, j;
  for(j = 0; j < nc; j++){
    for(i = 0; i < N; i++)
      y[i] += data[j*N+i] * x[indices[j*N+i]];
  }
}

function num_cols(csr_row, N)
{
  var temp, max = 0;
  for(var i = 0; i < N ; i++){
    temp = csr_row[i+1] - csr_row[i];
    if (max < temp)
      max = temp;
  }
  return max;
}

function csr_ell(csr_row, csr_col, csr_val, indices, data, nz, N){
  var i, j, k, temp, max = 0;
  for(i = 0; i < N; i++){
    k = 0;
    for(j = csr_row[i]; j < csr_row[i+1]; j++){
      data[k*N+i] = csr_val[j];
      indices[k*N+i] = csr_col[j];
      k++;
    }
  }
}

function num_diags(N,csr_row,csr_col)
{
  var ind = new Int32Array(2*N-1);
  var num_diag = 0;
  ind.fill(0);
  for(var i = 0; i < N ; i++){
    for(var j = csr_row[i]; j<csr_row[i+1]; j++){
      if(!ind[N+csr_col[j]-i-1]++)
        num_diag++;
    }
  }
  var diag_no = -(parseInt((2*N-1)/2));
  var min = Math.abs(diag_no);
  for(var i = 0; i < 2*N-1; i++){
    if(ind[i]){
      if(min > Math.abs(diag_no))
        min = Math.abs(diag_no); 
    }
    diag_no++; 
  }
  stride = N - min;
  return [num_diag,stride];
}

function spmv_dia(data, offset, N, nd, x, y, stride){
  var i, n, k, istart, iend,index;
  for(i = 0; i < nd; i++){
    k = offset[i];
    index = 0;
    istart = (0 < -k) ? (index = N-stride, -k) : 0;   
    iend = (N-1 < N-1-k) ? N-1 : N-1-k;
    for(n = istart; n <= iend; n++){
      y[n] += data[i*stride+n-index] * x[n+k];
    }
  }
}

function csr_dia(csr_row, csr_col, csr_val, offset, data, nz, N, stride){
  var ind = [], i, j, move;
  for(i = 0; i < 2*N-1; i++){
    ind[i] = 0; 
  }
  for(i = 0; i < N; i++){
    for(j = csr_row[i]; j < csr_row[i+1]; j++){ 
      ind[N+csr_col[j]-i-1]++;
    }
  }
  var diag_no = -(parseInt((2*N-1)/2));
  var index = 0;
  for(i = 0; i < 2*N-1; i++){
    if(ind[i])
      offset[index++] = diag_no;
    diag_no++; 
  }
  var c;
  
  for(i = 0; i < N; i++){
    for(j = csr_row[i]; j < csr_row[i+1]; j++){ 
      c = csr_col[j];  
      for(k = 0; k < offset.length; k++){
        move = 0;
        if(c - i == offset[k]){
          if(offset[k] < 0)
            move = N - stride; 
          data[k*stride+i-move] = csr_val[j];
          break;
        }
      }
    }
  }
  return stride;
}

function spmv_csr(row_ptr, csr_col, csr_val, N, nz, x, y){
  var temp;
  for(var i = 0; i < N; i++){
    temp = y[i];
    for(var j = row_ptr[i]; j < row_ptr[i+1]; j++){ 
      temp = temp + csr_val[j] * x[csr_col[j]];
    }
    y[i] = temp;
  }
}


function sort(start, end, array1, array2)
{ 
  var i, j, temp;
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

function coo_csr(row, col, val, N, nz, csr_row, csr_col, csr_val){
  var i;
  for(i = 0; i < nz; i++){
    csr_row[row[i]]++; 
  }

  var j = 0, j0 = 0;
  for(i = 0; i < N; i++){
    j0 = csr_row[i];
    csr_row[i] = j;
    j += j0;
  }

  var r, c, data;
  for(i = 0; i < nz; i++){
    j = csr_row[row[i]];
    csr_col[j] = col[i];
    csr_val[j] = val[i];
    csr_row[row[i]]++;
  }

  for(i = N-1; i > 0; i--){
    csr_row[i] = csr_row[i-1]; 
  }
  csr_row[0] = 0;
  csr_row[N] = nz;
  for(i = 0; i < N; i++)
    sort(csr_row[i], csr_row[i+1], csr_col, csr_val);
  
}


function spmv_coo(coo_row, coo_col, coo_val, N, nz, x, y){
  for(var i = 0; i < nz; i++){
    y[coo_row[i]] += coo_val[i] * x[coo_col[i]];
  }
}

  
var coo_mflops = -1, csr_mflops = -1, dia_mflops = -1, ell_mflops = -1;
var coo_sum=-1, csr_sum=-1, dia_sum=-1, ell_sum=-1;
var coo_sd=-1, csr_sd=-1, dia_sd=-1, ell_sd=-1;
var anz = 0;
var coo_flops = [], csr_flops = [], dia_flops = [], ell_flops = [];
var N;
var variance;
var inside = 0, inside_max = 100000;
function spmv(callback){
  var files = new Array(num);
  var fileno = 0;
  var loadfiles = function(fileno, callback){
    var request = new XMLHttpRequest();
    myname = filename + (Math.floor(fileno/10)).toString() + (fileno%10).toString() + '.mtx'
    console.log(myname);
    request.onreadystatechange = function() {
      if(request.readyState == 4 && request.status == 200){
        try{
        files[fileno] = request.responseText.split("\n");
        fileno++;
        if(fileno < num)
          loadfiles(fileno, callback);
        else {
          var start = 0;
          var symmetry, field;
          var row, col, val;
          var rows, cols, entries;
          var n = 0;
          for(var i = 0; i < num; i++){
            var temp = files[i];
            var index = 0;
            if(i == 0){
              var first = temp[0].split(" ");
              field = first[3];
              symmetry = first[4];
              while(temp[n][0] == "%")
                n++;
              info = temp[n++].split(" ");
              N = Number(info[0]);
              rows = Number(info[0]);
              cols = Number(info[1]);
              entries = Number(info[2]);
              console.log(rows, cols, entries);
              index = n;
              if(entries > Math.pow(2,27)){
                console.log("entries : cannot allocate this much");
                callback();
              }
              row = new Int32Array(entries);
              col = new Int32Array(entries);
              if(field != "pattern")
                val = new Float64Array(entries);
            }
            for(var j = start; index < temp.length - 1; index++){
              coord = temp[index].split(" ");
              row[j] = Number(coord[0]);
              col[j] = Number(coord[1]);
              if(symmetry == "symmetric"){
                if(field != "pattern"){
                  val[j] = Number(coord[2]);
                  if(val[j] < 0 || val[j] > 0){
                    if(row[j] == col[j])
                      anz++; 
                    else
                      anz = anz + 2;
                  }
                }
                else{
                  if(row[j] == col[j])
                    anz++; 
                  else
                    anz = anz + 2;
                } 
              }
              else{
                if(field != "pattern"){
                  val[j] = Number(coord[2]);
                  if(val[j] < 0 || val[j] > 0)
                    anz++;
                }
              }
              j++;
            }
            start = j;
          }
          if(anz == 0)
            anz = entries;
          console.log(anz);
          if(anz > Math.pow(2,27)){
            console.log("anz : cannot allocate this much");
            callback();
          }
          var coo_row = new Int32Array(anz), coo_col = new Int32Array(anz), coo_val = new Float64Array(anz), x = new Float64Array(cols), y = new Float64Array(rows);
          var csr_row = new Int32Array(rows+1), csr_col = new Int32Array(anz), csr_val = new Float64Array(anz), csr_y = new Float64Array(rows);
          var dia_y = new Float64Array(rows);
          var ell_y = new Float64Array(rows);
          var t1, t2, tt = 0.0;
          if(symmetry == "symmetric"){
            if(field == "pattern"){
              for(var i = 0, n = 0; n < start; n++) {
                coo_row[i] = Number(row[n] - 1);
                coo_col[i] = Number(col[n] - 1);
                coo_val[i] = 1.0;
                if(row[n] == col[n])
                  i++;
                else{
                  coo_row[i+1] = Number(col[n] - 1);
                  coo_col[i+1] = Number(row[n] - 1);
                  coo_val[i+1] = 1.0;
                  i = i + 2;
                }
              } 
            }
            else{
              for(var i = 0, n = 0; n < start; n++) {
                if(val[n] < 0 || val[n] > 0){
                  coo_row[i] = Number(row[n] - 1);
                  coo_col[i] = Number(col[n] - 1);
                  coo_val[i] = Number(val[n]);
                  if(row[n] == col[n])
                    i++;
                  else{
                    coo_row[i+1] = Number(col[n] - 1);
                    coo_col[i+1] = Number(row[n] - 1);
                    coo_val[i+1] = Number(val[n]);
                    i = i + 2;
                  }
                }
              }
            }
          }
          else{
            if(field == "pattern"){
              for(var i = 0, n = 0; n < start; n++, i++) {
                coo_row[i] = Number(row[n] - 1);
                coo_col[i] = Number(col[n] - 1);
                coo_val[i] = 1.0;
              }
            }
            else{
              for(var i = 0, n = 0; n < start; n++) {
                if(val[n] < 0 || val[n] > 0){
                  coo_row[i] = Number(row[n] - 1);
                  coo_col[i] = Number(col[n] - 1);
                  coo_val[i] = Number(val[n]);
                  i++;
                }
              }
            }
          }
          for(var i = 0; i < N; i++){
            x[i] = i;
            csr_row[i] = 0;
          } 
          if(anz > 1000000) inside_max = 1;
          else if (anz > 100000) inside_max = 10;
          else if (anz > 50000) inside_max = 50;
          else if(anz > 10000) inside_max = 100;
          else if(anz > 2000) inside_max = 1000;
          else if(anz > 100) inside_max = 10000;
          coo_csr(coo_row, coo_col, coo_val, N, anz, csr_row, csr_col, csr_val);
          for(var i = 0; i < 10; i++){
            csr_y.fill(0.0);
            t1 = performance.now();
            spmv_csr(csr_row, csr_col, csr_val, N, anz, x, csr_y);
            t2 = performance.now();
            csr_flops[i] = 1/Math.pow(10,6) * 2 * anz /((t2 - t1)/1000);
          }
          tt = 0.0
          for(var i = 0; i < 10; i++){
            csr_y.fill(0.0);
            t1 = performance.now();
            for(inside = 0; inside < inside_max; inside++)
              spmv_csr(csr_row, csr_col, csr_val, N, anz, x, csr_y);
            t2 = performance.now();
            csr_flops[i+10] = 1/Math.pow(10,6) * 2 * anz  * inside/((t2 - t1)/1000);
            tt = tt + t2 - t1;
          }
          tt = tt/1000; 
          csr_mflops = 1/Math.pow(10,6) * 2 * anz * 10 * inside / tt;
          variance = 0;
          for(var i = 0; i < 10; i++)
            variance += (csr_mflops - csr_flops[i+10]) * (csr_mflops - csr_flops[i+10]);
          variance /= 10;
          csr_sd = Math.sqrt(variance);
          csr_sum = parseInt(fletcher_sum(csr_y));
          console.log(parseInt(fletcher_sum(csr_y)));
  
          var result = num_diags(N, csr_row, csr_col);
          var nd = result[0]
          var stride = result[1]
          if(nd*stride < Math.pow(2,27)){
            var offset = new Int32Array(nd);
            var dia_data = new Float64Array(stride*nd);
            csr_dia(csr_row, csr_col, csr_val, offset, dia_data, anz, N, stride);
            for(var i = 0; i < 10; i++){
              dia_y.fill(0.0);
              t1 = performance.now();
              spmv_dia(dia_data, offset, N, nd, x, dia_y, stride);
              t2 = performance.now();
              dia_flops[i] = 1/Math.pow(10,6) * 2 * anz /((t2 - t1)/1000);
            }
            tt = 0.0
            for(var i = 0; i < 10; i++){
              dia_y.fill(0.0);
              t1 = performance.now();
              for(inside = 0; inside < inside_max; inside++)
                spmv_dia(dia_data, offset, N, nd, x, dia_y, stride);
              t2 = performance.now();
              dia_flops[i+10] = 1/Math.pow(10,6) * 2 * anz * inside/((t2 - t1)/1000);
              tt = tt + t2 - t1;
            }
            tt = tt/1000; 
            dia_mflops = 1/Math.pow(10,6) * 2 * anz * 10 * inside / tt;
            variance = 0;
            for(var i = 0; i < 10; i++)
              variance += (dia_mflops - dia_flops[i+10]) * (dia_mflops - dia_flops[i+10]);
            variance /= 10;
            dia_sd = Math.sqrt(variance);
            dia_sum = parseInt(fletcher_sum(dia_y));
            console.log(parseInt(fletcher_sum(dia_y)));
          }
          else
            console.log("cannot allocate for dia");
  
          var nc = num_cols(csr_row, N); 
          if(nc*N < Math.pow(2,27)){
            var indices = new Int32Array(nc*N), ell_data = new Float64Array(nc*N);
            csr_ell(csr_row, csr_col, csr_val, indices, ell_data, anz, N);
            for(var i = 0; i < 10; i++){
              ell_y.fill(0.0);
              t1 = performance.now();
              spmv_ell(ell_data, indices, N, nc, x, ell_y);
              t2 = performance.now();
              ell_flops[i] = 1/Math.pow(10,6) * 2 * anz /((t2 - t1)/1000);
            }
            tt = 0.0
            for(var i = 0; i < 10; i++){
              ell_y.fill(0.0);
              t1 = performance.now();
              for(inside = 0; inside < inside_max; inside++)
                spmv_ell(ell_data, indices, N, nc, x, ell_y);
              t2 = performance.now();
              ell_flops[i+10] = 1/Math.pow(10,6) * 2 * anz * inside/((t2 - t1)/1000);
              tt = tt + t2 - t1;
            }
            tt = tt/1000; 
            ell_mflops = 1/Math.pow(10,6) * 2 * anz * 10 * inside / tt;
            variance = 0;
            for(var i = 0; i < 10; i++)
              variance += (ell_mflops - ell_flops[i+10]) * (ell_mflops - ell_flops[i+10]);
            variance /= 10;
            ell_sd = Math.sqrt(variance);
            ell_sum = parseInt(fletcher_sum(ell_y));
            console.log(parseInt(fletcher_sum(ell_y)));
          }
          else
            console.log("cannot allocate for ell");
          for(var i = 0; i < 10; i++){
            y.fill(0.0);
            t1 = performance.now();
            spmv_coo(coo_row, coo_col, coo_val, N, anz, x, y);
            t2 = performance.now();
            coo_flops[i] = 1/Math.pow(10,6) * 2 * anz /((t2 - t1)/1000);
          }
          for(var i = 0; i < 10; i++){
            y.fill(0.0);
            t1 = performance.now();
            for(inside = 0; inside < inside_max; inside++)
              spmv_coo(coo_row, coo_col, coo_val, N, anz, x, y);
            t2 = performance.now();
            coo_flops[i+10] = 1/Math.pow(10,6) * 2 * anz * inside/((t2 - t1)/1000);
            tt = tt + t2 - t1;
          }
          tt = tt/1000; 
          coo_mflops = 1/Math.pow(10,6) * 2 * anz * 10 * inside / tt;
          variance = 0;
          for(var i = 0; i < 10; i++)
            variance += (coo_mflops - coo_flops[i+10]) * (coo_mflops - coo_flops[i+10]);
          variance /= 10;
          coo_sd = Math.sqrt(variance);
          console.log("coo_sd is " + coo_sd);
          coo_sum = parseInt(fletcher_sum(y));
          console.log(parseInt(fletcher_sum(y)));
          console.log("Done");
          callback();
        }
        }catch(e){
           console.log('Error : ', e.stack);
           callback();
         }
      }
    } 
    request.open('GET', myname, true);
    request.send();
  }
  loadfiles(fileno, function(){
    callback();
  });
}
