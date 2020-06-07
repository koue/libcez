#
all: pool assoc buffer string

pool:
	make -f Makefile.pool

assoc:
	make -f Makefile.assoc

buffer:
	make -f Makefile.buffer

string:
	make -f Makefile.string

clean:
	make -f Makefile.pool clean
	make -f Makefile.assoc clean
	make -f Makefile.buffer clean
	make -f Makefile.string clean

cleandir:
	make -f Makefile.pool cleandir
	make -f Makefile.assoc cleandir
	make -f Makefile.buffer cleandir
	make -f Makefile.string cleandir

install:
	make -f Makefile.pool install
	make -f Makefile.assoc install
	make -f Makefile.buffer install
	make -f Makefile.string install
