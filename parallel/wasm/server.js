// Imports
const shelljs = require("shelljs");
const fs = require("fs");
const express = require("express");
const path = require('path');
const url = require('url');
const colors = require("colors");
const app = express();

//Constants
let browserCmd;
let index_mat;
let result_csv;
let matrix_name;
let port_number;
let fileContents;
let float_precision;
let optimization_level;
let workers;
let total_matrices;
let single_file;
let skipped_files = fs.readFileSync("./skip.txt").toString().split("\n")
// Set server
app.use("/tmp", express.static('./tmp'));
app.use("/", express.static('./'));
app.use(express.static('./c'));
app.get('/result', (req, res) => {
    var query = url.parse(req.url).query;
    if (query) {
        obj = JSON.parse(decodeURIComponent(query));
        if(single_file) console.log(`Writing resul/.tts for ${matrix_name} to ${result_csv}`.green);
        let result = (obj.result)?obj.result.map((str)=>String(str).replace("\"","").replace("-nan","inf")):[];
        console.log(`STDOUT: ${result}`.america);
        if(single_file) fs.appendFileSync(result_csv, result.join()+"\n");
        res.send(obj);
        if(!single_file) compileNext();
        else{
            console.log("Done".green)
            process.exit(0);
        }
        return;
    }
});
module.exports.process_single = function({filename, browser, opt_level, num_workers, precision="double"}, port = 8080) {
    single_file = true;
    workers = num_workers;
    if(browser == 'chrome') browserCmd = `/mnt/local/cheetah/chrome73/opt/google/chrome/chrome http://localhost:${port}/`;
    else if(browser == 'firefox') browserCmd = `/mnt/cheetah/firefox65/firefox/firefox "http://localhost:${port}/"`;
    else {
        console.log(`Browser ${browser} not defined, please choose between chrome and firefox`.red);
        process.exit(1);
    }
      //Determine if precision single and double, used by compileNext() function
    if(precision !== "single"  && precision !=="double") {
        console.log(`Precision ${precision} not defined, please choose between double and single precision`.red);
        process.exit(1);
    }
    if(!shelljs.test("-f",filename))
    {
        console.log(`${filename} is not a valid path`.red);
        process.exit(1);
    }
    console.log("Running on browser: %s, precision %s, optimization flag -O%s".bgGreen.white, browser,precision, opt_level);
    float_precision = precision;
    optimization_level = opt_level;
    result_csv = `emcc-${browser}-${precision}-O${opt_level}-w${num_workers}-wasm-results.csv`;
    if(!shelljs.test("-f",result_csv)) fs.writeFileSync(result_csv, "");
    matrix_name = path.basename(filename);
    //Open server
    port_number = port;
    app.listen(port_number, () => {
        console.log(`Starting server on port ${port_number}!`.blue);
        let res = processMatrix(filename, workers);
        if (res.code === 1) process.exit(1);
    });
};
/**
 * In charge of server, retrieving matrices, and initiating collection process
 */
module.exports.process = function( {browser,precision="double", num_workers, opt_level}, port = 8080) {
    single_file = false;
    workers = num_workers;
    //Gather matrices by using find command
    let commandGetMtx = `find ./mini  -name "*.mtx"`;
    matrices = shelljs.exec(commandGetMtx, {silent:true}).stdout.split("\n");
    
	matrices.splice(matrices.length-1); //remove empty string
    //matrices = ["",]
    total_matrices = matrices.length;
    
    //Go through script options
    if(browser == 'chrome') browserCmd = `google-chrome http://localhost:${port}/`;
    else if(browser == 'firefox') browserCmd = `firefox -new-window "http://localhost:${port}/"`;
    else {
        console.log(`Browser ${browser} not defined, please choose between chrome and firefox`.red);
        process.exit(1);
    }
    //Determine if precision single and double, used by compileNext() function
    if(precision !== "single"  && precision !=="double") {
        console.log(`Precision ${precision} not defined, please choose between double and single precision`.red);
        process.exit(1);
    }
    console.log("Running on browser: %s, precision %s, optimization flag -O%s".bgGreen.white, browser,precision, opt_level);
    float_precision = precision;
    optimization_level = opt_level;
    result_csv = `emcc-${browser}-${precision}-O${opt_level}-w${num_workers}-wasm-results.csv`;
    if(!shelljs.test("-f",result_csv)) fs.writeFileSync(result_csv, "");
    //Open server
    port_number = port;
    app.listen(port_number, () => {
        console.log(`Starting server on port ${port_number}!`.blue);
        let res = compileNext();
        if (res.code === 1) process.exit(1);
    });  
};
/**
 * Helper functions
 */
//Compile next matrix using emscripten
function compileNext() {
    if(typeof index_mat=='undefined') index_mat = 0;
    else index_mat++;
    if( matrices.length === index_mat ) {
        console.log("Done Processing".green);
        process.exit(0);
    } 
    matrix_name = path.basename(matrices[index_mat]);
    fileContents = fs.readFileSync(result_csv).toString();
    //let code = shelljs.exec(`./number_zeros.out ${matrices[index_mat]}`,{silent:true}); 
    //console.log(Number(code.stdout));
    //if(Number(code.stdout) === -1 || 
    if(fileContents.indexOf(matrix_name) !== -1 || skipped_files.indexOf(matrix_name)!== -1) {
	return compileNext();
    }
    console.log(`Processing file : ${matrix_name}, ${(index_mat + 1)}/${total_matrices}`.magenta);
    return  processMatrix(matrices[index_mat], workers);
}

function processMatrix(matrix_path, num_workers) {

    let matrix_name = path.basename(matrix_path);
    console.log(matrix_name);
    let commandCompile = `emcc -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=6 -s WASM=1 -s TOTAL_MEMORY=1GB -s WASM_MEM_MAX=1GB -s NO_EXIT_RUNTIME=0 -o ./run.js -s "EXPORTED_FUNCTIONS=['_main','_mainTrial']" --preload-file ${matrix_path}@${matrix_name} -s  ABORTING_MALLOC=0 -s ASSERTIONS=1 -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["ccall","cwrap"]' --no-heap-copy  -m64 -O${optimization_level}  ${(float_precision == "double")?"":"-D FLOAT"} -I./c/include ./c/main.c ./c/spmv_coo.c ./c/spmv_csr.c  ./c/spmv_dia.c ./c/spmv_ell.c ./c/spmv_diaii.c ./c/spmv_ellii.c ./c/conversions.c ./c/utils.c ./c/mmio.c -lm`;
    console.log(commandCompile.yellow);
    let res = shelljs.exec(commandCompile, {silent:true});
    if (res.code === 1) {
        errorMatrix(matrix_name);
    }
    fs.writeFileSync('./tmp/args.js',`var matrix_name = '${matrix_name}'; var workers = '${num_workers}';`);
    shelljs.exec(browserCmd);
    return res;
}
//Log error in matrix
function errorMatrix(matrix_name) {
    console.error("Error compiling matrix, ", matrix_name);
    fs.appendFileSync("./error_compile.txt",matrix_name);
}
