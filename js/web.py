import os
import subprocess
import sys
import json

from bottle import route, run, static_file

@route('/static/<name>')
def get_file(name):
  response = static_file(name, root=os.getcwd())
  response.set_header('Cache-Control', 'no-cache, max-age=0')
  #response.set_header('Expires', '0')
  #response.set_header('Pragma', 'no-cache')
  return response

@route('/result/<json_string>')
def result(json_string):
  parsed_json = json.loads(json_string) 
  output_file = parsed_json['output_file']
  f = open(os.path.join(os.getcwd(), output_file),'a')
  browser = parsed_json['browser']
  print browser
  f.write(parsed_json['file'])
  f.write(",")
  f.write(str(parsed_json['N']))
  f.write(",")
  f.write(str(parsed_json['nnz']))
  f.write(",")
  f.write(str(parsed_json['coo']))
  f.write(",")
  f.write(str(parsed_json['coo_sum']))
  f.write(",")
  f.write(str(parsed_json['csr']))
  f.write(",")
  f.write(str(parsed_json['csr_sum']))
  f.write(",")
  f.write(str(parsed_json['dia']))
  f.write(",")
  f.write(str(parsed_json['dia_sum']))
  f.write(",")
  f.write(str(parsed_json['ell']))
  f.write(",")
  f.write(str(parsed_json['ell_sum']))
  f.write(",")
  #f.write("======")
  #for i in range(100):
    #f.write(str(parsed_json['coo_flops'][i]))
    #f.write(",")
  #f.write("======")
  #for i in range(100):
    #f.write(str(parsed_json['csr_flops'][i]))
    #f.write(",")
  #if parsed_json['dia'] > 0 :
    #for i in range(3):
      #f.write(str(parsed_json['dia_flops'][i]))
      #f.write(",")
  #else :
      #f.write("-1,-1,-1,") 
  #if parsed_json['ell'] > 0 :
    #for i in range(3):
      #f.write(str(parsed_json['ell_flops'][i]))
      #f.write(",")
  #else :
      #f.write("-1,-1,-1,") 
  f.write("\n")
  f.close()
  if browser == 0:
    subprocess.call(['killall', '-9', 'chrome']);
  elif browser == 1:
    subprocess.call(['killall', '-9', 'firefox']);
run(host='localhost', port=8080, quiet=True)
