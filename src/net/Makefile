#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=		ceznet
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=	-Wall -Wstrict-prototypes -I${.CURDIR}
SRCS=		atomicio.c http.c cez_net_http.h
INCS=		cez_net_http.h
NO_OBJ=		yes

.include <bsd.lib.mk>
