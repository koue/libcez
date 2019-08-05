#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=		cezmydb
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=	-Wall -Wstrict-prototypes -I${.CURDIR} \
		-I${LOCALBASE}/include \
		-I${.CURDIR}/../fossil/base \
		-I${.CURDIR}/../queue
SRCS=		mydb.c cez_mydb.h
INCS=		cez_mydb.h
NO_OBJ=		yes

.include <bsd.lib.mk>
