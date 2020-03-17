SHELL := '/bin/bash'
CC = gcc

INCLUDES = -I./include -I../../../../papi/src

SRCS = main.c spmv_coo.c spmv_csr.c spmv_dia.c spmv_diaii.c spmv_ell.c spmv_ellii.c conversions.c utils.c mmio.c

OBJS = $(SRCS:.c=.o)
ASMS = $(SRCS:.c=.s)

LIBS = -L../../../../lib -lm -lrt -lpthread -lpapi -lpfm

CFLAGS = -m64 -march=native -mtune=native -O3 -Wno-unused-result

OUT=run

float: CFLAGS += -DFLOAT
float: $(OBJS) $(ASMS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)_$@ $(OBJS) $(LIBS)
	rm -f *.o

double: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)_$@ $(OBJS) $(LIBS)
	rm -f *.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $< 

%.s: %.c
	$(CC) $(CFLAGS) -S $(INCLUDES) $< 

clean:
	-rm -f run *.o *.s
