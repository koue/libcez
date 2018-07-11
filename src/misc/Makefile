#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezmisc
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
SRCS=           test.c hmac.c cez_misc.h
INCS=           cez_misc.h
NO_OBJ=         yes

.include <bsd.lib.mk>
