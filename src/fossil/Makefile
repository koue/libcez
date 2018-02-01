#
LIB=            cezfossil
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
CFLAGS+=	-I/usr/local/include
LDFLAGS+=	-L/usr/local/lib
LDADD=		-lsqlite3
SRCS=           blob.c cgi.c cson_amalgamation.c db.c printf.c \
		cez-cson_amalgamation.h cez-fossil.h
INCS=           cez-cson_amalgamation.h cez-fossil.h
NO_OBJ=         yes

.include <bsd.lib.mk>
