#
all: http net

http:
	make -f Makefile.http

net:
	make -f Makefile.net

clean:
	make -f Makefile.http clean
	make -f Makefile.net clean

cleandir:
	make -f Makefile.http cleandir
	make -f Makefile.net cleandir

install:
	make -f Makefile.http install
	make -f Makefile.net install
