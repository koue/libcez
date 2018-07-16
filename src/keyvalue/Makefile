#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezkv
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
SRCS=           kv.c cez_kv.h
INCS=           cez_kv.h
NO_OBJ=         yes

.include <bsd.lib.mk>
