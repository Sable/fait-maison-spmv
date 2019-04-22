#!/usr/bin/env node
var argv = require('yargs')
    .usage("Usage: $0 -b ['firefox'|'chrome'] -p ['single'|'double'] -f [path-to-file] -o [optimization-level] -w [num-workers")
    .demandOption(['b','p','o', 'w'])
    .alias('f','path')
    .alias('b','browser')
    .alias('p','precision')
    .alias('o','optimization-level')
    .alias('w','num-workers')
    .argv;

const serve = require("./server.js");

if(argv.b !== 'firefox' && argv.b !== 'chrome') {
    console.log("Browser %s not supported", argv.b);
    process.exit(1);    
}else if(argv.p !== 'single' && argv.p !== 'double') {
    console.log("Format %s not supported", argv.p);
    process.exit(1);
}
let opts;
if(!argv.f)
{
     opts = {"browser":argv.b,"precision":argv.p,"opt_level":argv.o,"num_workers":argv.w};
     serve.process(opts,8081);
}else{
    opts = {"browser":argv.b,"precision":argv.p,"opt_level":argv.o,"num_workers":argv.w,"filename":argv.f};
    serve.process_single(opts,8081);
}
