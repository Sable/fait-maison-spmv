#ifndef __CONFIG_H
  #define __CONFIG_H
  #ifdef FLOAT
    #define MYTYPE float
    #define FORMAT "%f"
    #define MYEPSILON FLT_EPSILON
  #else
    #define MYTYPE double
    #define FORMAT "%lf"
    #define MYEPSILON DBL_EPSILON
  #endif
#endif
