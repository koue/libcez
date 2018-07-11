#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezprayer
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR}
SRCS=		assoc.c \
		buffer.c \
		html.c \
		misc.c \
		mystring.c \
		pool.c \
		str.c \
		template.c \
		template_compile.c \
		template_expand.c \
		template_parse.c \
		template_vals.c \
		utf8.c \
		cez_prayer.h \
		cez_prayer_assoc.h \
		cez_prayer_buffer.h \
		cez_prayer_html.h \
		cez_prayer_misc.h \
		cez_prayer_mystring.h \
		cez_prayer_pool.h \
		cez_prayer_str.h \
		cez_prayer_template_fns.h \
		cez_prayer_template_structs.h \
		cez_prayer_template_vals.h \
		cez_prayer_utf8.h

INCS=		cez_prayer.h \
		cez_prayer_assoc.h \
		cez_prayer_buffer.h \
		cez_prayer_html.h \
		cez_prayer_misc.h \
		cez_prayer_mystring.h \
		cez_prayer_pool.h \
		cez_prayer_str.h \
		cez_prayer_template_fns.h \
		cez_prayer_template_structs.h \
		cez_prayer_template_vals.h \
		cez_prayer_utf8.h

NO_OBJ=         yes

.include <bsd.lib.mk>
