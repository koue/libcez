#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezmisc
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -fPIC -I${.CURDIR}
SRCS=           hmac.c time.c url.c cez_misc.h
INCS=           cez_misc.h
NO_OBJ=         yes

.include <bsd.lib.mk>
