#
CFLAGS  = -I../../../src/prayer

TYPE=html

TARGETS=templates.a

T_FILES=	footer.t \
		header.t \
		status.t \
		tb_icon.t \
		toolbar.t \
		toolbar_left.t \
		toolbar_right.t \
		welcome.t \
		welcome_help.t

O_FILES=_template_index.o ${T_FILES:.t=.o}

C_FILES=${O_FILES:.o=.c}

.PRECIOUS: $(C_FILES)

HTML=		welcome.html

EXPAND=		../src/template_expand
COMPILE=	../src/template_compile

all: $(TARGETS)

test: $(HTML)

templates.a: $(O_FILES)
	rm -f templates.a
	ar q templates.a $(O_FILES)

_template_index.c:
	../src/build_index.pl $(TYPE) $(T_FILES) > _template_index.c

.SUFFIXES: .t
.t.c: ${.PREFIX}.t Makefile
	$(COMPILE) $(TYPE) $@ $*

clean:
	rm -f $(TARGETS) *.html *.o *.c \#*\# *~

include Makefile.deps

