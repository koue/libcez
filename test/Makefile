#
CC=		cc -Wall -Wstrict-prototypes -g -O0

CFLAGS+=	-I../src/config \
		-I../src/fossil \
		-I../src/misc
CFLAGS+=	-I/usr/local/include

LDFLAGS+=	-L/usr/local/lib

LDADD=		-lsqlite3 \
		-lcrypto \
		../src/config/libcezconfig.a \
		../src/fossil/libcezfossil.a \
		../src/misc/libcezmisc.a

CONF_DEP=	../src/config/cez-config.h \
		../src/config/libcezconfig.a

FOSSIL_DEP=	../src/fossil/cez-cson_amalgamation.h \
		../src/fossil/cez-fossil.h \
		../src/fossil/libcezfossil.a

MISC_DEP=	../src/misc/cez-misc.h \
		../src/misc/libcezmisc.a


all: libtest jsontest cgitest configarraytest configqueuetest hmactest

libtest: lib-test.c $(FOSSIL_DEP) $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o lib-test lib-test.c $(LDADD)

jsontest: json-test.c $(FOSSIL_DEP) $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o json-test json-test.c $(LDADD)

cgitest: cgi-test.c $(FOSSIL_DEP) $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o cgi-test cgi-test.c $(LDADD)

configarraytest: config-array-test.c $(CONFIG_DEP) $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o config-array-test config-array-test.c $(LDADD)

configqueuetest: config-queue-test.c $(CONFIG_DEP) $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o config-queue-test config-queue-test.c $(LDADD)

hmactest: hmac-test.c $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o hmac-test hmac-test.c $(LDADD)

# keep this empty otherwise installation fails
install:

test:
	./json-test
	./lib-test
	QUERY_STRING='show=test&go=pass' ./cgi-test
	./config-array-test
	./config-queue-test
	./hmac-test

clean:
	rm -f *-test *.core
