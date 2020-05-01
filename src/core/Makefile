#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezcore
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
SRCS=		pool.c \
		cez_core.h
.ifndef CORE_POOL_ONLY
SRCS+=		misc.c \
		str.c
.else
CFLAGS+= -DCORE_POOL_ONLY
.endif

INCS=		cez_core.h

NO_OBJ=         yes

.include <bsd.lib.mk>
