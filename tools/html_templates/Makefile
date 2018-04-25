#
TEMPLATES = html

SUBDIRS = src $(TEMPLATES)

CFLAGS  = -I../../src/prayer/

all: index.o
	for i in $(SUBDIRS); do $(MAKE) -C $$i all; done

install:
	for i in $(SUBDIRS); do $(MAKE) -C $$i install; done

clean:
	for i in $(SUBDIRS); do $(MAKE) -C $$i clean; done
	rm -f index.o index.c

index.o: index.c
	$(CC) $(CFLAGS) -c index.c

index.c: ./src/build_map_index.pl
	./src/build_map_index.pl $(TEMPLATES) > index.c

