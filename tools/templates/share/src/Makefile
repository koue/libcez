#
CC= 		cc -Wall -Wstrict-prototypes -g -O0 -static
CFLAGS+=	-I../../../../src/prayer
LDADD=		-L../../../../src/prayer -lcezprayer

all: template_expand template_compile

template_expand: template_expand_main.o log.o empty.o
	$(CC) $(CFLAGS) -o template_expand log.o empty.o template_expand_main.o $(LDADD)

template_compile: template_compile_main.o log.o empty.o
	$(CC) $(LFLAGS) -o template_compile log.o empty.o template_compile_main.o $(LDADD)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $<

install:

clean:
	rm -f template_expand template_compile *.o *~ \#*\#

