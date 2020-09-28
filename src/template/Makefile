#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            ceztemplate
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes \
		-I${.CURDIR} \
		-I${.CURDIR}/../core \
		-I${LOCALBASE}/include

SRCS=		html.c \
		template.c \
		template_compile.c \
		template_expand.c \
		template_parse.c \
		template_vals.c \
		utf8.c \
		cez_template.h \
		cez_template_fns.h \
		cez_template_html.h \
		cez_template_structs.h \
		cez_template_utf8.h \
		cez_template_vals.h

INCS=		cez_template.h \
		cez_template_fns.h \
		cez_template_html.h \
		cez_template_structs.h \
		cez_template_utf8.h \
		cez_template_vals.h

NO_OBJ=         yes

.include <bsd.lib.mk>
