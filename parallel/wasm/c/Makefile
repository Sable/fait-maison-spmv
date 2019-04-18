SHELL := '/bin/bash'
CC = gcc
EMCC = emcc

INCLUDES = -I./include -I/mnt/cheetah/papi/src

SRCS = main.c spmv_coo.c spmv_csr.c spmv_dia.c spmv_diaii.c spmv_ell.c spmv_ellii.c conversions.c utils.c mmio.c

OBJS = $(SRCS:.c=.o)
ASMS = $(SRCS:.c=.s)

LIBS = -L/mnt/cheetah/lib -lm -lrt -lpthread -lpapi -lpfm

CFLAGS = -m64 -O3 $(TEST)
EMCCFLAGS = $(TEST) -m64 -O3 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=6 -s WASM=1 -s TOTAL_MEMORY=1GB -s  ALLOW_MEMORY_GROWTH=1 --no-heap-copy -s WASM_MEM_MAX=1GB -s NO_EXIT_RUNTIME=0 --preload-file $(FILE)
OUTPUT = program.html

run: $(OBJS) $(ASMS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJS) $(LIBS)

web: $(SRCS)
	$(EMCC) $(SRCS) $(EMCCFLAGS) $(INCLUDES) -o $(OUTPUT) 

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $< 

%.s: %.c
	$(CC) $(CFLAGS) -S $(INCLUDES) $< 

clean:
	-rm -f run *.o *.s
