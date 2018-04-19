#
LIB=            cezprayer
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
SRCS=		assoc.c buffer.c misc.c mystring.c pool.c str.c utf8.c \
		cez_prayer.h cez_prayer_assoc.h cez_prayer_misc.h \
		cez_prayer_pool.h cez_prayer_str.h \
		template.c template_compile.c template_expand.c \
		template_parse.c template_vals.c
INCS=           cez_prayer.h cez_prayer_assoc.h cez_prayer_misc.h \
		cez_prayer_pool.h cez_prayer_str.h
NO_OBJ=         yes

.include <bsd.lib.mk>
