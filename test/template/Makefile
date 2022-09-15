#
LOCALBASE?=	/usr/local
PROGS=		template_test

CFLAGS=		-I${.CURDIR}/../../src/core \
		-I${.CURDIR}/../../src/template \
		-I${.CURDIR}/../

LDFLAGS+=	-L${.CURDIR}/../../src/core \
		-L${.CURDIR}/../../src/template \
		-L${LOCALBASE}/lib

LDADD=		-lceztemplate -lcezcorepool -lcezcorebuffer -lcezcorestring \
		-lcezcoreassoc -lutil

MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR= .

# keep this empty otherwise installation fails
install:

test:
	./template_test

.include <bsd.progs.mk>
