#
CC=cc -Wall -Wstrict-prototypes -g -O0 -I./ -I/usr/local/include

all: libcez.a

blob.o: blob.c cez.h
	$(CC) -c blob.c

cgi.o: cgi.c cez.h
	$(CC) -c cgi.c

configfile.o: configfile.c cez.h
	$(CC) -c configfile.c

config-array.o: config-array.c cez.h
	$(CC) -c config-array.c

config-queue.o: config-queue.c cez.h
	$(CC) -c config-queue.c

cson_amalgamation.o: cson_amalgamation.c cson_amalgamation.h
	$(CC) -c cson_amalgamation.c

db.o: db.c cez.h
	$(CC) -c db.c

printf.o: printf.c cez.h
	$(CC) -c printf.c

test.o: test.c cez.h
	$(CC) -c test.c

libcez.a: blob.o cgi.o configfile.o config-array.o config-queue.o \
	cson_amalgamation.o db.o printf.o test.o
	ar rc libcez.a blob.o cgi.o configfile.o config-array.o config-queue.o \
		cson_amalgamation.o db.o printf.o test.o
	ranlib libcez.a

install:
	cp cez.h /usr/local/include/
	cp cson_amalgamation.h /usr/local/include/
	cp libcez.a /usr/local/lib/

clean:
	rm -f libcez.a *.o
