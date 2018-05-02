#
CFLAGS  = -I../../src/prayer/

SUBDIRS = share/src $(TEMPLATES)

all: index.o
.	for i in $(SUBDIRS)
		$(MAKE) -C $(i) TEMPLATES=$(i)
.	endfor

clean:
.	for i in $(SUBDIRS)
		$(MAKE) -C $(i) clean
.	endfor

	rm -f index.o index.c

index.o: index.c
	$(CC) $(CFLAGS) -c index.c

index.c: ./share/src/build_map_index.pl
	./share/src/build_map_index.pl $(TEMPLATES) > index.c

