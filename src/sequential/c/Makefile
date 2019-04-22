SHELL := '/bin/bash'
CC = gcc

INCLUDES = -I./include

SRCS = main.c spmv_csr.c spmv_coo.c spmv_dia.c spmv_ell.c conversions.c utils.c mmio.c
OBJS = $(SRCS:.c=.o)
ASMS = $(SRCS:.c=.s)

FEATURES_SRCS = features.c mmio.c utils.c
FEATURES_OBJS = $(FEATURES_SRCS:.c=.o)

LIBS = -lm

CFLAGS = -m64 -O3 -Wno-unused-result

DEBUG ?= 0
ifeq ($(DEBUG), 1)
  CFLAGS += -DDEBUG 
  LIBS += -L/mnt/cheetah/lib -lpapi -lpfm
  INCLUDES += -I/mnt/cheetah/papi/src
endif

OUT=run

float: CFLAGS += -DFLOAT
float: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)_$@ $(OBJS) $(LIBS)
	rm -f *.o

double: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)_$@ $(OBJS) $(LIBS)
	rm -f *.o

features: $(FEATURES_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)_$@ $(FEATURES_OBJS) $(LIBS)
	rm -f *.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $< 

%.s: %.c
	$(CC) $(CFLAGS) -S $(INCLUDES) $< 

clean:
	-rm -f run_* *.o *.s

