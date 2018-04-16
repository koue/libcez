#
CC=		cc -Wall -Wstrict-prototypes -g -O0

CFLAGS+=	-I../src/config \
		-I../src/fossil \
		-I../src/misc \
		-I../src/prayer
CFLAGS+=	-I/usr/local/include

LDFLAGS+=	-L/usr/local/lib

LDADD=		-lsqlite3 \
		-lcrypto \
		../src/config/libcezconfig.a \
		../src/fossil/libcezfossil.a \
		../src/misc/libcezmisc.a \
		../src/prayer/libcezprayer.a

CONF_DEP=	../src/config/cez_config.h \
		../src/config/libcezconfig.a

FOSSIL_DEP=	../src/fossil/cez_cson_amalgamation.h \
		../src/fossil/cez_fossil.h \
		../src/fossil/libcezfossil.a

MISC_DEP=	../src/misc/cez_misc.h \
		../src/misc/libcezmisc.a

PRAYER_DEP=	../src/prayer/cez_prayer.h \
		../src/prayer/libcezprayer.a


all: libtest jsontest cgitest configarraytest configqueuetest hmactest assoctest

libtest: lib_test.c $(FOSSIL_DEP) $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o lib_test lib_test.c $(LDADD)

jsontest: json_test.c $(FOSSIL_DEP) $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o json_test json_test.c $(LDADD)

cgitest: cgi_test.c $(FOSSIL_DEP) $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o cgi_test cgi_test.c $(LDADD)

configarraytest: config_array_test.c $(CONFIG_DEP) $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o config_array_test config_array_test.c $(LDADD)

configqueuetest: config_queue_test.c $(CONFIG_DEP) $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o config_queue_test config_queue_test.c $(LDADD)

hmactest: hmac_test.c $(MISC_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o hmac_test hmac_test.c $(LDADD)

assoctest: assoc_test.c $(PRAYER_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o assoc_test assoc_test.c $(LDADD)

# keep this empty otherwise installation fails
install:

test:
	./json_test
	./lib_test
	QUERY_STRING='show=test&go=pass' ./cgi_test
	./config_array_test
	./config_queue_test
	./hmac_test
	./assoc_test

clean:
	rm -f *_test *.core
