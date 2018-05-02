#
CFLAGS  = -I../../../src/prayer

TARGETS=templates.a

O_FILES=_template_index.o ${T_FILES:.t=.o}

C_FILES=${O_FILES:.o=.c}

.PRECIOUS: $(C_FILES)

EXPAND=		../share/src/template_expand
COMPILE=	../share/src/template_compile

all: $(TARGETS)

test: $(HTML)

templates.a: $(O_FILES)
	rm -f templates.a
	ar q templates.a $(O_FILES)

_template_index.c:
	../share/src/build_index.pl $(TEMPLATES) $(T_FILES) > _template_index.c

.SUFFIXES: .t .html
.t.c: ${.PREFIX}.t Makefile
	$(COMPILE) $(TEMPLATES) $@ $*

.t.html: ${.PREFIX}.t Makefile common.vars ${.PREFIX}.vars
	$(EXPAND) ${.TARGET} ${.PREFIX} common.vars ${.PREFIX}.vars

clean:
	rm -f $(TARGETS) *.html *.o *.c \#*\# *~

