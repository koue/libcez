#
LOCALBASE?=	/usr/local
PROGS=		blob_test \
		printf_test

CFLAGS=		-I${.CURDIR}/../ \
		-I${.CURDIR}/../../src/fossil/base

LDFLAGS+=	-L${.CURDIR}/../../src/fossil/base

LDADD.blob_test=	-lcezfossilbase
LDADD.printf_test=	-lcezfossilbase

.ifndef NOSQLITE
# fossil cson requires sqlite
PROGS+=		cson_test \
		db_test
CFLAGS+=	-I${.CURDIR}/../../src/fossil/cson \
		-I${.CURDIR}/../../src/fossil/db \
		-I${LOCALBASE}/include
LDFLAGS+=	-L${.CURDIR}/../../src/fossil/cson \
		-L${.CURDIR}/../../src/fossil/db \
		-L${LOCALBASE}/lib
LDADD.cson_test=-lcezfossilbase -lcezfossilcson -lsqlite3
LDADD.db_test=	-lcezfossilbase -lcezfossildb -lsqlite3
.endif

MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR= .

# keep this empty otherwise installation fails
install:

test:
	./blob_test
.ifndef NOSQLITE
	./cson_test
	./db_test
.endif
	./printf_test

.include <bsd.progs.mk>
