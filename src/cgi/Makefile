#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=		cezcgi
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=	-Wall -Wstrict-prototypes -I${.CURDIR}/../core -I${.CURDIR}
SRCS=		cez_cgi.h cgi.c
INCS=		cez_cgi.h
NO_OBJ=		yes

.include <bsd.lib.mk>
