#
CC=cc -Wall -Wstrict-prototypes -g -O0 -I../src -I/usr/local/include -I../src -L../src -L/usr/local/lib

all: libtest jsontest cgitest configarraytest configqueuetest

libtest: lib-test.c ../src/cez.h ../src/libcez.a
	$(CC) -o lib-test lib-test.c -lsqlite3 ../src/libcez.a

jsontest: json-test.c ../src/cez.h ../src/libcez.a
	$(CC) -o json-test json-test.c -lsqlite3 ../src/libcez.a

cgitest: cgi-test.c ../src/cez.h ../src/libcez.a
	$(CC) -o cgi-test cgi-test.c ../src/libcez.a

configarraytest: config-array-test.c ../src/cez.h ../src/libcez.a
	$(CC) -o config-array-test config-array-test.c ../src/libcez.a

configqueuetest: config-queue-test.c ../src/cez.h ../src/libcez.a
	$(CC) -o config-queue-test config-queue-test.c ../src/libcez.a

# keep this empty otherwise installation fails
install:

test:
	./json-test
	./lib-test
	QUERY_STRING='show=test&go=pass' ./cgi-test
	./config-array-test
	./config-queue-test

clean:
	rm -f *-test *.core
