#
LOCALBASE?=	/usr/local
PROGS=		template_test

CFLAGS=		-I${.CURDIR}/../../src/core \
		-I${.CURDIR}/../../src/net \
		-I${.CURDIR}/../../src/prayer \
		-I${.CURDIR}/../

LDFLAGS+=	-L${.CURDIR}/../../src/core \
		-L${.CURDIR}/../../src/prayer

LDADD=		-lcezprayer -lcezcorepool -lcezcorebuffer -lcezcorestring -lcezcoreassoc -lutil -liconv

.ifndef NOSSL
# libressl
CFLAGS+=	-DUSE_SSL=1 \
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
	./template_test

.include <bsd.progs.mk>
