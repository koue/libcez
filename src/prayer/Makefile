#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=            cezprayer
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=        -Wall -Wstrict-prototypes -I${.CURDIR} \
		-I${.CURDIR}/../core -I${LOCALBASE}/include
SRCS=		assoc.c \
		buffer.c \
		config.c \
		html.c \
		iostream.c \
		ipaddr.c \
		list.c \
		log.c \
		mystring.c \
		os.c \
		process.c \
		template.c \
		template_compile.c \
		template_expand.c \
		template_parse.c \
		template_vals.c \
		utf8.c \
		cez_prayer.h \
		cez_prayer_assoc.h \
		cez_prayer_buffer.h \
		cez_prayer_config.h \
		cez_prayer_html.h \
		cez_prayer_iostream.h \
		cez_prayer_ipaddr.h \
		cez_prayer_list.h \
		cez_prayer_log.h \
		cez_prayer_mystring.h \
		cez_prayer_os.h \
		cez_prayer_os_bsd.h \
		cez_prayer_process.h \
		cez_prayer_ssl.h \
		cez_prayer_template_fns.h \
		cez_prayer_template_structs.h \
		cez_prayer_template_vals.h \
		cez_prayer_utf8.h
.ifdef NOSSL
SRCS+=		no_ssl.c
.else
SRCS+=		ssl.c
.endif

INCS=		cez_prayer.h \
		cez_prayer_assoc.h \
		cez_prayer_buffer.h \
		cez_prayer_config.h \
		cez_prayer_html.h \
		cez_prayer_iostream.h \
		cez_prayer_ipaddr.h \
		cez_prayer_list.h \
		cez_prayer_log.h \
		cez_prayer_mystring.h \
		cez_prayer_os.h \
		cez_prayer_os_bsd.h \
		cez_prayer_process.h \
		cez_prayer_ssl.h \
		cez_prayer_template_fns.h \
		cez_prayer_template_structs.h \
		cez_prayer_template_vals.h \
		cez_prayer_utf8.h

NO_OBJ=         yes

.include <bsd.lib.mk>
