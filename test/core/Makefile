#
LOCALBASE?=	/usr/local
PROGS=		assoc_test \
		pool_test \
		str_test

CFLAGS=		-I${.CURDIR}/../../src/core \
		-I${.CURDIR}/../

LDFLAGS+=	-L${.CURDIR}/../../src/core

LDADD=		-lcezcorepool -lcezcoreassoc -lcezcorebuffer -lutil

MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR= .

# keep this empty otherwise installation fails
install:

test:
	./assoc_test
	./pool_test
	./str_test

.include <bsd.progs.mk>
