#
LOCALBASE?=	/usr/local
PROGS=		pool_test

CFLAGS=		-I${.CURDIR}/../../src/core \
		-I${.CURDIR}/../

LDFLAGS+=	-L${.CURDIR}/../../src/core

LDADD=		-lcezcore -lutil

MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR= .

# keep this empty otherwise installation fails
install:

test:
	./pool_test

.include <bsd.progs.mk>
