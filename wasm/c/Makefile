SHELL := '/bin/bash'
CC = emcc $(EMSCRIPTEN)
EMSCRIPTEN = -s WASM=1 -o $@.js -s "EXPORTED_FUNCTIONS=['_main','_mainTrial']" --preload-file rgg_n_2_21_s0.mtx -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 --no-heap-copy 
INCLUDES = -I./include

SRCS = main.c spmv_csr.c spmv_coo.c spmv_dia.c spmv_ell.c conversions.c utils.c mmio.c

OBJS = $(SRCS:.c=.o)
ASMS = $(SRCS:.c=.s)

LIBS = -lm

CFLAGS = -m64 -O3 $(TEST) 

run:
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $< 

%.s: %.c
	$(CC) $(CFLAGS) -S $(INCLUDES) $< 

clean:
	-rm -f run *.o *.s *.data *.mtx *.wasm *.js
