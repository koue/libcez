#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=		cezrender
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=	-Wall -Wstrict-prototypes -I${.CURDIR}
SRCS=		cez_render.h render.c
INCS=		cez_render.h
NO_OBJ=		yes

.include <bsd.lib.mk>
