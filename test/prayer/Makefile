#
LOCALBASE?=	/usr/local
PROGS=		assoc_test \
		os_test \
		template_test

CFLAGS=		-I${.CURDIR}/../../src/core \
		-I${.CURDIR}/../../src/prayer \
		-I${.CURDIR}/../

LDFLAGS+=	-L${.CURDIR}/../../src/core \
		-L${.CURDIR}/../../src/prayer

LDADD=		-lcezprayer -lcezcore -lutil -liconv

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
	./template_test

.include <bsd.progs.mk>
