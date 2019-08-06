#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezfossilbase
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
SRCS=		blob.c printf.c cez_fossil_base.h
INCS=           cez_fossil_base.h
NO_OBJ=         yes

.include <bsd.lib.mk>
