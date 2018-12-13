#
PROGS=		assoc_test \
		cgi_test \
		config_array_test \
		config_queue_test \
		hmac_test \
		json_test \
		kv_test \
		lib_test \
		queue_test \
		time_test \
		url_test

CFLAGS=		-I../src/config \
		-I../src/fossil \
		-I../src/keyvalue \
		-I../src/misc \
		-I../src/queue \
		-I../src/prayer \
		-I/usr/local/include

LDFLAGS+=	-L../src/config \
		-L../src/fossil \
		-L../src/keyvalue \
		-L../src/misc \
		-L../src/queue \
		-L../src/prayer \
		-L/usr/local/lib

LDADD.assoc_test=	-lcezprayer -lcezmisc
LDADD.cgi_test=		-lcezfossil -lcezmisc
LDADD.config_array_test=-lcezconfig -lcezmisc
LDADD.config_queue_test=-lcezconfig -lcezmisc
LDADD.hmac_test=	-lcrypto -lcezmisc
LDADD.json_test=	-lcezfossil -lsqlite3 -lcezmisc
LDADD.kv_test=		-lcezkv -lcezmisc
LDADD.lib_test=		-lcezfossil -lsqlite3 -lcezmisc
LDADD.queue_test=	-lcezqueue
LDADD.time_test=	-lcezmisc
LDADD.url_test=		-lcezmisc

MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR= .

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
	./kv_test
	./queue_test
	./time_test
	./url_test

.include <bsd.progs.mk>
