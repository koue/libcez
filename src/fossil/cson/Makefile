#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezfossilcson
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
CFLAGS+=	-I${LOCALBASE}/include
LDFLAGS+=	-L${LOCALBASE}/lib
LDADD=		-lsqlite3
SRCS=		cson_amalgamation.c cez_fossil_cson.h
INCS=		cez_fossil_cson.h
NO_OBJ=         yes

.include <bsd.lib.mk>
