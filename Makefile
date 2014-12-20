CC?=cc
LD=$(CC)

PROJECT=mysql-hll

OBJS=main.o
VPATH=src

CFLAGS?=-O2

build: mysql-hll.so.1.0.1

mysql-hll.so.1.0.1: $(OBJS) deps/hll/lib/libhyperloglog.a
	$(CC) -shared \
	    -Wl,-install_name,mysql-hll.so \
	    -static \
	    -Ldeps/hll/lib -lhyperloglog \
	    -o mysql-hll.so $(OBJS) -lc

deps/hll/lib/libhyperloglog.a:
	make -C deps/hll lib/libhyperloglog.a

.c.o:
	$(CC) -c -fPIC -std=c90 -g -Wall -Wconversion $(CFLAGS) src/$*.c

clean:
	rm -f *.o *.so.*
	rm -rf deps/*/bin deps/*/lib
