#
PROGS=		assoc_test \
		hmac_test \
		json_test \
		kv_test \
		lib_test \
		mydb_test \
		queue_test \
		time_test \
		url_test

CFLAGS=		-I../src/fossil \
		-I../src/keyvalue \
		-I../src/misc \
		-I../src/mydb \
		-I../src/queue \
		-I../src/prayer \
		-I/usr/local/include

LDFLAGS+=	-L../src/fossil \
		-L../src/keyvalue \
		-L../src/misc \
		-L../src/mydb \
		-L../src/queue \
		-L../src/prayer \
		-L/usr/local/lib \
		-L/usr/local/lib/mysql

LDADD.assoc_test=	-lcezprayer -lcezmisc
LDADD.hmac_test=	-lcrypto -lcezmisc
LDADD.json_test=	-lcezfossil -lsqlite3 -lcezmisc
LDADD.kv_test=		-lcezkv -lcezmisc
LDADD.lib_test=		-lcezfossil -lsqlite3 -lcezmisc
LDADD.mydb_test=	-lmariadb -lcezmydb -lcezqueue -lcezmisc
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
	./hmac_test
	./assoc_test
	./kv_test
#	./mydb_test
	./queue_test
	./time_test
	./url_test

.include <bsd.progs.mk>
