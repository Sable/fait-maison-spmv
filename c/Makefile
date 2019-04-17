SHELL := '/bin/bash'
CC = gcc

INCLUDES = -I./include

SRCS = main.c spmv_csr.c spmv_coo.c spmv_dia.c spmv_ell.c conversions.c utils.c mmio.c

OBJS = $(SRCS:.c=.o)
ASMS = $(SRCS:.c=.s)

LIBS = -lm

CFLAGS = -m64 -O3 -Wno-unused-result 

float: ARG = -DFLOAT
float:  $(OBJS)
	$(CC) $(CFLAGS) $(ARG) $(INCLUDES) -o run_$@ $(OBJS) $(LIBS)
	rm -f *.o

double: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o run_$@ $(OBJS) $(LIBS)
	rm -f *.o

%.o: %.c
	$(CC) $(CFLAGS) $(ARG) $(INCLUDES) -c -o $@ $< 

%.s: %.c
	$(CC) $(CFLAGS) -S $(INCLUDES) $< 

clean:
	-rm -f run_* *.o *.s

