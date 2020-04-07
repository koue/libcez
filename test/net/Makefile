#
LOCALBASE?=	/usr/local
PROGS=		http_test

CFLAGS=		-I${.CURDIR}/../../src/net \
		-I${.CURDIR}/../../src/prayer \
		-I${.CURDIR}/../

LDFLAGS+=	-L${.CURDIR}/../../src/net \
		-L${.CURDIR}/../../src/prayer

LDADD=		-lceznet -lcezprayer -lutil

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

.include <bsd.progs.mk>
