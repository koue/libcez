#
LOCALBASE?=	/usr/local
PROGS=		http_test \
		os_test

CFLAGS=		-I${.CURDIR}/../../src/core \
		-I${.CURDIR}/../../src/net \
		-I${.CURDIR}/../

LDFLAGS+=	-L${.CURDIR}/../../src/core \
		-L${.CURDIR}/../../src/net

LDADD=-lceznethttp -lceznet -lcezcorepool -lcezcorebuffer -lcezcorestring -lcezcoreassoc -lutil
.ifndef NOSSL
# libressl
CFLAGS+=	-DSSL=1 \
		-I${LOCALBASE}/include
LDFLAGS+=	-L${LOCALBASE}/lib
LDADD+=		-lssl -lcrypto
.endif


MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR= .

# keep this empty otherwise installation fails
install:

test:
	./http_test
	./os_test

.include <bsd.progs.mk>
