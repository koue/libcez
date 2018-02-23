#
LIB=            cezprayer
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
#CFLAGS+=	-I/usr/local/include
#LDFLAGS+=	-L/usr/local/lib
#LDADD=		-lsqlite3
SRCS=		assoc.c misc.c pool.c str.c \
		cez-prayer.h cez-prayer-assoc.h cez-prayer-misc.h \
		cez-prayer-pool.h cez-prayer-str.h
INCS=           cez-prayer.h cez-prayer-assoc.h cez-prayer-misc.h \
		cez-prayer-pool.h cez-prayer-str.h
NO_OBJ=         yes

.include <bsd.lib.mk>
