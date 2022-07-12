#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=		cezqueue
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=	-Wall -Wstrict-prototypes -fPIC -I${.CURDIR}
SRCS=		configfile.c queue.c cez_queue.h
INCS=		cez_queue.h
NO_OBJ=		yes

.include <bsd.lib.mk>
