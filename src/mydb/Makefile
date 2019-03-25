#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=		cezmydb
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=	-Wall -Wstrict-prototypes -I${.CURDIR} -I${.CURDIR}/../queue
CFLAGS+=	-I${LOCALBASE}/include
LDFLAGS+=	-L${LOCALBASE}/lib/mysql \
		-L${.CURDIR}/../queue
LDADD=		-lmariadb -lcezqueue
SRCS=		mydb.c cez_mydb.h
INCS=		cez_mydb.h
NO_OBJ=		yes

.include <bsd.lib.mk>
