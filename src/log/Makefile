#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezlog
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR} -I${.CURDIR}/../core
SRCS=		log.c cez_log.h

INCS=		cez_log.h

NO_OBJ=         yes

.include <bsd.lib.mk>
