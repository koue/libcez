#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezconfig
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
SRCS=           configfile.c config_array.c config_queue.c cez_config.h
INCS=           cez_config.h
NO_OBJ=         yes

.include <bsd.lib.mk>
