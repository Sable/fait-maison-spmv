SHELL := '/bin/bash'
CC = gcc

INCLUDES = -I./include

SRCS = main.c spmv_csr.c spmv_coo.c spmv_dia.c spmv_ell.c conversions.c utils.c mmio.c

OBJS = $(SRCS:.c=.o)
ASMS = $(SRCS:.c=.s)

LIBS = -lm

CFLAGS = -m64 -O3 $(TEST) 

run: $(OBJS) $(ASMS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $< 

%.s: %.c
	$(CC) $(CFLAGS) -S $(INCLUDES) $< 

clean:
	-rm -f run *.o *.s
