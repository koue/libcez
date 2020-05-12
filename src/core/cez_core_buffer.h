/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#ifndef _CEZ_CORE_BUFFER
#define _CEZ_CORE_BUFFER
#pragma once

/* Buffer
 *
 * Arbitary length block of data with simple sequential access methods:
 *
 *   Append single character or block of data to end of buffer
 *   Rewind read offset to start of "file".
 *   Fetch single character or block of data from read position
 *
 * Typically small (< 8Kbytes). Should optimise for small buffers.
 *
 *  Methods:
 *    addchar    (add single character to end)
 *    addblock   (add arbitary block of data to end)
 *    seek       (seek to position in buffer)
 *    current    (current offset)
 *    length     (current length of buffer)
 *    getchar    (fetch single character from current position, advance)
 *    getblock   (fetch block of data from current position, advance)
 *
 * Propose:
 *   Implement using pools and linked lists to start off with
 *   Come back and redo using backing store files if this all works out.
 */

/* Initial implantation attempt is linked list of blocks, all same size
 * Pretty simple minded, should be okay for sequential access */

struct msgblock {
    struct msgblock *next;
    unsigned char data[1];
};

struct buffer {
    /* Global information */
    struct pool *pool;          /* Memory allocated from this pool */
    unsigned long size;         /* Size of entire file */
    unsigned long offset;       /* Offset into file for read methods */
    unsigned long blocksize;    /* Size of individual blocks */

    /* Block information for append methods */
    struct msgblock *first;     /* First block in linked list */
    struct msgblock *last;      /* Final block in linked list: append here */
    unsigned long avail;        /* Space available in last block */

    /* Block information for fetch methods */
    struct msgblock *fetch;     /* Current block for read access */
    unsigned long fetch_avail;  /* (Potential) unfetched data from block */
};

/* 8 Kbytes minus size of "next" ptr for linked list */
#define PREFERRED_BUFFER_BLOCK_SIZE (8192-sizeof(struct msgblock *))

struct buffer *buffer_create(struct pool *p, unsigned long blocksize);
void buffer_free(struct buffer *b);
void buffer_reset(struct buffer *b);

unsigned long buffer_size(struct buffer *b);

void buffer_putchar(struct buffer *b, unsigned char c);

void buffer_vaprintf(struct buffer *b, char *format, va_list ap);
void buffer_printf(struct buffer *b, char *format, ...);
void buffer_puts(struct buffer *b, char *string);
void buffer_vaprintf_translate(struct buffer *b, char *fmt, va_list ap);
void buffer_printf_translate(struct buffer *b, char *fmt, ...);
void buffer_puts_translate(struct buffer *b, char *t);

/* Fetch methods */
void buffer_rewind(struct buffer *b);
int buffer_getchar(struct buffer *b);
BOOL buffer_seek_offset(struct buffer *b, unsigned long offset);
void *buffer_fetch(struct buffer *b,
                   unsigned long offset, unsigned long count, BOOL copy);

BOOL
buffer_fetch_block(struct buffer *b,
                   unsigned char **ptrp, unsigned long *sizep);

/* Macros */

#define bgetc(b)  (((b->offset < b->size) && (b->fetch_avail > 0)) ?   \
                   (b->offset++,                                       \
                    b->fetch->data[b->blocksize-(b->fetch_avail--)]) : \
                   buffer_getchar(b))

#define bputc(b, c)                             \
do {                                            \
  unsigned char _c = (unsigned char)c;          \
                                                \
  if (b->avail > 0) {                           \
    b->last->data[b->blocksize-b->avail] = _c;  \
    b->avail--; b->size++;                      \
  } else                                        \
    buffer_putchar(b, _c);                      \
} while (0)

#define bputs(a, b) buffer_puts(a, (char *)(b))
#define bprintf     buffer_printf

void buffer_encode_url(struct buffer *b, char *s);
void buffer_encode_canon(struct buffer *b, char *s);

#endif

