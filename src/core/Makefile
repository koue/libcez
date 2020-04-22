#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezcore
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
SRCS=		pool.c str.c misc.c cez_core.h

INCS=		cez_core.h

NO_OBJ=         yes

.include <bsd.lib.mk>
