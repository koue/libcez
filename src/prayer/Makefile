#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezprayer
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes \
		-I${.CURDIR} \
		-I${.CURDIR}/../core \
		-I${.CURDIR}/../net \
		-I${LOCALBASE}/include

SRCS=		config.c \
		html.c \
		list.c \
		log.c \
		template.c \
		template_compile.c \
		template_expand.c \
		template_parse.c \
		template_vals.c \
		utf8.c \
		cez_prayer.h \
		cez_prayer_config.h \
		cez_prayer_html.h \
		cez_prayer_list.h \
		cez_prayer_log.h \
		cez_prayer_template_fns.h \
		cez_prayer_template_structs.h \
		cez_prayer_template_vals.h \
		cez_prayer_utf8.h

INCS=		cez_prayer.h \
		cez_prayer_config.h \
		cez_prayer_html.h \
		cez_prayer_list.h \
		cez_prayer_log.h \
		cez_prayer_template_fns.h \
		cez_prayer_template_structs.h \
		cez_prayer_template_vals.h \
		cez_prayer_utf8.h

NO_OBJ=         yes

.include <bsd.lib.mk>
