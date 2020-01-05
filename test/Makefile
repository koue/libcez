#
LOCALBASE?=	/usr/local
PROGS=		assoc_test \
		blob_test \
		hmac_test \
		kv_test \
		printf_test \
		queue_test \
		time_test \
		url_test \
		util_test

CFLAGS=		-I../src/fossil/base \
		-I../src/keyvalue \
		-I../src/misc \
		-I../src/queue \
		-I../src/prayer \
		-I../src/util

LDFLAGS+=	-L../src/fossil/base \
		-L../src/keyvalue \
		-L../src/misc \
		-L../src/queue \
		-L../src/prayer \
		-L../src/util

LDADD.assoc_test=	-lcezprayer -lcezmisc
LDADD.blob_test=	-lcezfossilbase -lcezmisc
LDADD.hmac_test=	-lcrypto -lcezmisc
LDADD.kv_test=		-lcezkv -lcezmisc
LDADD.printf_test=	-lcezfossilbase -lcezmisc
LDADD.queue_test=	-lcezqueue
LDADD.time_test=	-lcezmisc
LDADD.url_test=		-lcezmisc
LDADD.util_test=	-lcezutil

.ifndef NOSQLITE
# fossil cson requires sqlite
PROGS+=		cson_test db_test
CFLAGS+=	-I../src/fossil/cson \
		-I../src/fossil/db \
		-I${LOCALBASE}/include
LDFLAGS+=	-L../src/fossil/cson \
		-L../src/fossil/db \
		-L${LOCALBASE}/lib
LDADD.cson_test=-lcezfossilbase -lcezfossilcson -lsqlite3 -lcezmisc
LDADD.db_test=	-lcezfossilbase -lcezfossildb -lsqlite3 -lcezmisc
.endif

.ifndef NOMYDB
PROGS+=		mydb_test
CFLAGS+=	-I../src/mydb \
		-I${LOCALBASE}/include
LDFLAGS+=	-L../src/mydb \
		-L${LOCALBASE}/lib/mysql
LDADD.mydb_test=-lmariadb -lcezmydb -lcezfossilbase -lcezqueue -lcezmisc
.endif

MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR= .

# keep this empty otherwise installation fails
install:

test:
	./assoc_test
	./blob_test
.ifndef NOSQLITE
	./cson_test
	./db_test
.endif
	./hmac_test
	./kv_test
#	./mydb_test
	./printf_test
	./queue_test
	./time_test
	./url_test
	./util_test

.include <bsd.progs.mk>
