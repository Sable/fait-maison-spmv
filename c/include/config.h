#ifndef __CONFIG_H
  #define __CONFIG_H
  #ifdef FLOAT
    #define MYTYPE float
    #define FORMAT "%f"
  #else
    #define MYTYPE double
    #define FORMAT "%lf"
  #endif
#endif
