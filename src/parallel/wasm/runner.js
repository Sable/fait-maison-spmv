(()=>{
    var result_arr = [];
  
    Module = {
        'print': function(text) { 
                    console.log(text);
                    text = text.replace(",","");
                    if(!isNaN(text)) text = Number(text);
                    result_arr.push(text);
                },
        'printErr': function(text) { console.log('stderr: ' + text) }
      };
    Module.noExitRuntime = true;
    Module.noInitialRun  = true; 
    Module.onRuntimeInitialized = ()=>{
        console.log("Processing: ", matrix_name); //matrix_name defined in args.js by server.js in compilerNext(), check index.html and server.js
        result_arr.push(matrix_name);
        runner(matrix_name, workers).then((res)=>{            
            var result_arr_str = JSON.stringify({result:result_arr});
            console.log(result_arr_str);
            var xmlhttp = new XMLHttpRequest();
            xmlhttp.open("GET", "/result?" + result_arr_str, false);
            xmlhttp.send();
            window.close();
        });
    
    };
    function runner(name_matrix, worker)
    {
        return new Promise((resolve,reject)=>{
            try{
                Module.ccall('mainTrial','number',['string','string','string'],[name_matrix,'1', worker]);
                console.log(result_arr);
            }catch(e){
                result_arr.push(-1,-1,-1,-1,-1,-1,-1,-1);
                console.log("CATCHING STATEMENT, format 1, error: ",e);
            }
            try{
                Module.ccall('mainTrial','number',['string','string','string'],[name_matrix,'2',worker]);
                console.log(result_arr);
            }catch(e){
                result_arr.push(-1,-1,-1);
                console.log("CATCHING STATEMENT, format 2, error: ",e);
            }
            try{
                Module.ccall('mainTrial','number',['string','string', 'string'],[name_matrix,'4', worker]);
                console.log(result_arr);
            }catch(e){
                result_arr.push(-1,-1,-1);
                console.log("CATCHING STATEMENT, format 4, error: ",e); //Process ell first because there is a weird wasm bug, where if dia fails, ell will fails, even if ell runs fine on its own
            }
            try{
                var code = Module.ccall('mainTrial','number',['string','string', 'string'],[name_matrix,'3', worker]);
                console.log(result_arr);
            }catch(e){
                result_arr.push(-1,-1,-1);
                console.log("CATCHING STATEMENT, format 3, error: ",e);
            } 
            result_arr.splice(0,1);
            let ell = result_arr.splice(12,3);
            result_arr.push(...ell);
            console.log(result_arr);
            try{
                var code = Module.ccall('mainTrial','number',['string','string', 'string'],[name_matrix,'5', worker]);
                console.log(result_arr);
            }catch(e){
                result_arr.push(-1,-1,-1);
                console.log("CATCHING STATEMENT, format 5, error: ",e);

            }
            try{
                var code = Module.ccall('mainTrial','number',['string','string', 'string'],[name_matrix,'6', worker]);
                console.log(result_arr);
            }catch(e){
                result_arr.push(-1,-1,-1);
                console.log("CATCHING STATEMENT, format 6, error: ",e);

            }
            resolve(result_arr); 
        });
    }
    
    
})();
