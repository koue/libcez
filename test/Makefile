#
SUBDIR= 	core \
		net \
		template

LOCALBASE?=	/usr/local
PROGS=		cgi_test \
		hmac_test \
		kv_test \
		log_test \
		queue_test \
		render_test \
		time_test \
		url_test \
		util_test

CFLAGS= 	-Werror \
		-I${.CURDIR}/../src/cgi \
		-I${.CURDIR}/../src/core \
		-I${.CURDIR}/../src/keyvalue \
		-I${.CURDIR}/../src/log \
		-I${.CURDIR}/../src/misc \
		-I${.CURDIR}/../src/queue \
		-I${.CURDIR}/../src/render \
		-I${.CURDIR}/../src/util

LDFLAGS+=	-L${.CURDIR}/../src/cgi \
		-L${.CURDIR}/../src/core \
		-L${.CURDIR}/../src/keyvalue \
		-L${.CURDIR}/../src/log \
		-L${.CURDIR}/../src/misc \
		-L${.CURDIR}/../src/queue \
		-L${.CURDIR}/../src/render \
		-L${.CURDIR}/../src/util

LDADD.cgi_test=		-lcezcgi -lcezcorepool
LDADD.hmac_test=	-lcezmisc -lcrypto
LDADD.kv_test=		-lcezkv -lcezmisc
LDADD.log_test=		-lcezlog -lcezcorepool
LDADD.queue_test=	-lcezqueue
LDADD.render_test=	-lcezrender
LDADD.time_test=	-lcezmisc
LDADD.url_test=		-lcezmisc
LDADD.util_test=	-lcezutil

.ifndef NOSSL
# libressl
CFLAGS+=	-I${LOCALBASE}/include
LDFLAGS+=	-L${LOCALBASE}/lib
.endif

MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR= .

# keep this empty otherwise installation fails
install:

test:
	./cgi_test
	./hmac_test
	./kv_test
	./log_test
	./queue_test
	./render_test
	./time_test
	./url_test
	./util_test

.include <bsd.progs.mk>
.include <bsd.subdir.mk>
