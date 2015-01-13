CC?=cc
LD=$(CC)

PROJECT=mysql-hll

OBJS=main.o
VPATH=src

CFLAGS?=-O2

build: mysql-hll.so

mysql-hll.so: $(OBJS) deps/hll/lib/libhyperloglog.a
	$(CC) \
	    -lc \
	    -shared \
	    -o mysql-hll.so $(OBJS) deps/hll/lib/libhyperloglog.a

deps/hll/lib/libhyperloglog.a:
	make -C deps/hll lib/libhyperloglog.a

.c.o:
	$(CC) -c -fPIC -ansi -g -Wall -Wconversion -D_DEFAULT_SOURCE $(CFLAGS) `mysql_config --include` src/$*.c

clean:
	rm -f *.o *.so.*
	make -C deps/hll clean

install: build
	install mysql-hll.so "`mysql_config --plugindir`"
