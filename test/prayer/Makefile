#
LOCALBASE?=	/usr/local
PROGS=		assoc_test \
		os_test \
		pool_test

CFLAGS=		-I../../src/prayer \
		-I../

LDFLAGS+=	-L../../src/prayer

LDADD=		-lcezprayer -lutil

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
	./assoc_test
	./os_test
	./pool_test

.include <bsd.progs.mk>
