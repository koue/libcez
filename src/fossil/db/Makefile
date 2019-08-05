#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezfossildb
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR} -I${.CURDIR}/../base
CFLAGS+=	-I${LOCALBASE}/include
LDFLAGS+=	-L${LOCALBASE}/lib
LDADD=		-lsqlite3
SRCS+=		db.c cez_fossil_db.h
INCS=           cez_fossil_db.h
NO_OBJ=         yes

.include <bsd.lib.mk>
