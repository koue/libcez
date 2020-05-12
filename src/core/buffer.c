/* Copyright (c) 2018-2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_core_pool.h"
#include "cez_core_buffer.h"

/* Class for processing arbitary length strings with append facility and
 * linear search/read access. The name "buffer" is historical, its possible
 * that this should be called something else today. However I can't really
 * face renaming several thousand references to "buffer" right now */

/* ====================================================================== */

/* buffer_create() ******************************************************
 *
 * Create a new buffer structure.
 *      pool: Target pool for storage
 * blocksize: Preferred size for individual allocation blocks.
 *            Typically quite large for IO operations, small if we
 *            have a good feel for like upper bound on size of object.
 *            0 => Picks default which would appropriate for IO objects
 *
 * Returns: New buffer object
 ***********************************************************************/

struct buffer *buffer_create(struct pool *pool, unsigned long blocksize)
{
    struct buffer *b = pool_alloc(pool, sizeof(struct buffer));

    b->size = 0;                /* Buffer starts out empty */
    b->pool = pool;             /* Allocate from this pool  */
    b->blocksize =
        (blocksize > 0) ? blocksize : PREFERRED_BUFFER_BLOCK_SIZE;
    b->first = NIL;
    b->last = NIL;
    b->avail = 0;               /* Forces allocation */

    b->offset = 0;              /* State used by read methods */
    b->fetch = NIL;
    b->fetch_avail = 0;

    return (b);
}

/* buffer_free() ********************************************************
 *
 * Free buffer including all allocated blocks. NOOP if pool defined.
 *    b: buffer to free
 ***********************************************************************/

void buffer_free(struct buffer *b)
{
    struct msgblock *current, *next;

    if (b->pool)                /* Noop if data allocated from pool */
        return;

    for (current = b->first; current; current = next) {
        next = current->next;
        free(current);
    }
    free(b);
}

/* buffer_reset() *******************************************************
 *
 * Wipe existing buffer so that caller can overwrite existing data
 ***********************************************************************/

void buffer_reset(struct buffer *b)
{

    b->size = 0;                /* Buffer starts out empty */
    b->first = NIL;
    b->last = NIL;
    b->avail = 0;               /* Forces allocation */

    b->offset = 0;              /* State used by read methods */
    b->fetch = NIL;
    b->fetch_avail = 0;
}

/* buffer_size() ********************************************************
 *
 * Returns number of characters currently stored in buffer
 ***********************************************************************/

unsigned long buffer_size(struct buffer *b)
{
    return (b->size);
}

/* ====================================================================== */

/* put/extend methods */

/* buffer_add_msgblock() ************************************************
 *
 * Adds a new msgblock (historical name for allocation space) to buffer:
 * gives us some room to expand.
 *   b: Buffer to extend.
 ***********************************************************************/

static void buffer_add_msgblock(struct buffer *b)
{
    struct msgblock *mb;

    mb = pool_alloc(b->pool, sizeof(struct msgblock) + (b->blocksize) - 1);
    mb->next = NIL;

    if (b->first) {
        b->last->next = mb;     /* Add msgblock to end of chain */
        b->last = mb;
    } else
        b->first = b->last = mb;        /* First msgblock in chain */
}

/* buffer_putchar() *****************************************************
 *
 * Add a single character to the end of the buffer, extending buffer if
 * required. Typically access via macro bputc().
 *   b: Buffer
 *   c: Character to add
 ***********************************************************************/

void buffer_putchar(struct buffer *b, unsigned char c)
{
    /* Space available in current msgblock */
    if (b->avail > 0) {
        b->last->data[b->blocksize - b->avail] = c;
        b->avail--;
        b->size++;
        return;
    }

    /* Need to allocate a fresh msgblock */
    buffer_add_msgblock(b);
    b->avail = b->blocksize - 1;
    b->last->data[0] = c;
    b->size++;
}

/* buffer_print_ulong() *************************************************
 *
 * Print number (as decimal represention) at end of buffer.
 *     b: Buffer
 * value: value
 ***********************************************************************/

static void buffer_print_ulong(struct buffer *b, unsigned long value)
{
    unsigned long tmp, weight;

    /* All numbers contain at least one digit.
     * Find weight of most significant digit. */
    for (weight = 1, tmp = value / 10; tmp > 0; tmp /= 10)
        weight *= 10;

    for (tmp = value; weight > 0; weight /= 10) {
        if (value >= weight) {  /* Strictly speaking redundant... */
            bputc(b, '0' + (value / weight));   /* Digit other than zero */
            value -= weight * (value / weight); /* Calculate remainder */
        } else
            bputc(b, '0');
    }
}

/* buffer_print_hex() ***************************************************
 *
 * Print number (as hex represention) at end of buffer.
 *     b: Buffer
 * value: value
 ***********************************************************************/

static void buffer_print_hex(struct buffer *b, unsigned long value)
{
    unsigned long tmp, weight;

    /* All numbers contain at least one digit.
     * Find weight of most significant digit. */
    for (weight = 1, tmp = value / 16; tmp > 0; tmp /= 16)
        weight *= 16;

    for (tmp = value; weight > 0; weight /= 16) {
        unsigned long digit = value / weight;
        unsigned char c =
            (digit > 9) ? ('a' + (digit - 10)) : ('0' + digit);

        bputc(b, c);

        value -= weight * digit;
    }
}

/* buffer_vaprintf() ****************************************************
 *
 * vaprintf equivalent for buffer
 *     b: Buffer
 *   fmt: vaprintf format string, followed by arguments.
 ***********************************************************************/

void buffer_vaprintf(struct buffer *b, char *fmt, va_list ap)
{
    unsigned char *s, c;

    while ((c = *fmt++)) {
        if (c != '%') {
            bputc(b, c);
        } else
            switch (*fmt++) {
            case 's':          /* string */
                if ((s = (unsigned char *) va_arg(ap, char *))) {
                    while ((c = *s++))
                        bputc(b, c);
                } else
                    bputs(b, "(nil)");
                break;
            case 'l':
                if (*fmt == 'u') {
                    buffer_print_ulong(b, va_arg(ap, unsigned long));
                    fmt++;
                } else if (*fmt == 'x') {
                    buffer_print_hex(b, va_arg(ap, unsigned long));
                    fmt++;
                } else
                    buffer_print_ulong(b, va_arg(ap, long));
                break;
            case 'd':
                if (*fmt == 'u') {
                    buffer_print_ulong(b, va_arg(ap, unsigned int));
                    fmt++;
                } else
                    buffer_print_ulong(b, va_arg(ap, int));
                break;
            case 'c':
                bputc(b, (unsigned char) va_arg(ap, int));
                break;
            case 'x':
                buffer_print_hex(b, va_arg(ap, unsigned long));
                break;
            case '%':
                bputc(b, '%');
                break;
            default:
                /* log_fatal("Bad format string to buffer_printf"); */
		fprintf(stderr, "Bad format string to buffer_printf\n");
		exit (1);
            }
    }
}

/* buffer_printf() ******************************************************
 *
 * printf equivalent for buffer. Typically accessed via bprintf() macro
 *     b: Buffer
 *   fmt: vaprintf format string, followed by arguments.
 ***********************************************************************/

void buffer_printf(struct buffer *b, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    buffer_vaprintf(b, fmt, ap);
    va_end(ap);
}

/* buffer_printf() ******************************************************
 *
 * puts equivalent for buffer. Typically accessed via bputs() macro
 *     b: Buffer
 *     t: String to print
 ***********************************************************************/

void buffer_puts(struct buffer *b, char *t)
{
    unsigned char *s = (unsigned char *) t;
    char c;

    if (!s)
        bputs(b, "(nil)");
    else
        while ((c = *s++))
            bputc(b, c);
}

/* ====================================================================== */

/* buffer_vaprintf_translate() ******************************************
 *
 * Print string translating '/' characters with '@'. Used by short URL
 * translation stuff.
 *     b: Buffer
 *   fmt: vaprintf format string, followed by arguments.
 ***********************************************************************/

void buffer_vaprintf_translate(struct buffer *b, char *fmt, va_list ap)
{
    unsigned char *s, c;

    while ((c = *fmt++)) {
        switch (c) {
        case '%':
            switch (*fmt++) {
            case 's':          /* string */
                if ((s = (unsigned char *) va_arg(ap, char *))) {
                    while ((c = *s++))
                        bputc(b, (c == '/') ? '@' : c);
                } else
                    bputs(b, "(nil)");
                break;
            case 'l':
                if (*fmt == 'u') {
                    buffer_print_ulong(b, va_arg(ap, unsigned long));
                    fmt++;
                } else
                    buffer_print_ulong(b, va_arg(ap, long));
                break;
            case 'd':
                if (*fmt == 'u') {
                    buffer_print_ulong(b, va_arg(ap, unsigned int));
                    fmt++;
                } else
                    buffer_print_ulong(b, va_arg(ap, int));
                break;
            case 'c':
                bputc(b, (unsigned char) va_arg(ap, int));
                break;
            case '%':
                bputc(b, '%');
                break;
            default:
                /* log_fatal("Bad format string to buffer_printf"); */
                fprintf(stderr, "Bad format string to buffer_printf\n");
		exit (1);
            }
            break;
        case '/':
            bputc(b, '@');
            break;
        default:
            bputc(b, c);
        }
    }
}

/* buffer_printf_translate() ********************************************
 *
 * Print string translating '/' characters with '@'. Used by short URL
 * translation stuff.
 *     b: Buffer
 *   fmt: vaprintf format string, followed by arguments.
 ***********************************************************************/

void buffer_printf_translate(struct buffer *b, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    buffer_vaprintf_translate(b, fmt, ap);
    va_end(ap);
}

/* buffer_puts_translate() ***********************************************
 *
 * Print string translating '/' characters with '@'. Used by short URL
 * translation stuff.
 *     b: Buffer
 *     t: String to print/translate
 ***********************************************************************/

void buffer_puts_translate(struct buffer *b, char *t)
{
    unsigned char *s = (unsigned char *) t;
    char c;

    if (!s)
        bputs(b, "(nil)");
    else
        while ((c = *s++))
            bputc(b, (c == '/') ? '@' : c);
}

/* ====================================================================== */

/* Fetch methods */

/* buffer_rewind() ******************************************************
 *
 * Rewind read access ptrs to start of the buffer
 ***********************************************************************/

void buffer_rewind(struct buffer *b)
{
    b->offset = 0;
    b->fetch = b->first;
    b->fetch_avail = b->blocksize;
}

/* buffer_seek_offset() *************************************************
 *
 * Seek to given offset in buffer
 *       b: Buffer
 *  offset: Offset into buffer
 *
 * Returns: T on sucess. NIL if offset is out of range.
 ***********************************************************************/

BOOL buffer_seek_offset(struct buffer *b, unsigned long offset)
{
    struct msgblock *mb = b->first;

    if ((b->offset = offset) > b->size)
        return (NIL);

    while (offset > b->blocksize) {
        mb = mb->next;
        offset -= b->blocksize;
    }

    b->fetch = mb;              /* Correct block */
    b->fetch_avail = b->blocksize - offset;     /* Data left in this block */

    return (T);
}

/* buffer_getchar() *****************************************************
 *
 * Get character from current read location in buffer. Usually used via
 * bgetc() macro.
 *  b: Buffer
 ***********************************************************************/

int buffer_getchar(struct buffer *b)
{
    unsigned char result;

    if (b->offset >= b->size)   /* Nothing more available */
        return (EOF);

    if (b->fetch == NIL)        /* Need to set up fetch ptrs */
        buffer_rewind(b);

    if (b->fetch_avail == 0) {
        b->fetch = b->fetch->next;      /* Next block in chain */
        b->fetch_avail = b->blocksize;
    }

    /* Record current character */
    result = b->fetch->data[b->blocksize - b->fetch_avail];

    /* Then update pointers */
    b->offset++;
    b->fetch_avail--;

    return ((int) result);
}

/* ====================================================================== */

/* buffer_getblock() *****************************************************
 *
 * Get block of characters from buffer. Static support fn for buffer_fetch
 *      b: Buffer
 *  block: Target location
 *  count: Number of characters.
 ***********************************************************************/

static unsigned long
buffer_getblock(struct buffer *b, void *block, unsigned long count)
{
    char *s = (char *) block;
    unsigned long result;

    if (b->offset >= b->size)   /* No more bytes available */
        return (0);

    if (count > (b->size - b->offset))
        count = b->size - b->offset;    /* Only this many bytes available */

    result = count;             /* Return (adjusted) count to caller */

    if (b->fetch == NIL)        /* Need to set up fetch ptrs */
        buffer_rewind(b);

    b->offset += count;

    if (count < b->fetch_avail) {
        /* Can fetch block from current bucket */
        memcpy(s, &(b->fetch->data[b->blocksize - b->fetch_avail]), count);

        b->fetch_avail -= count;
        return (result);
    }

    /* Otherwise block fetch will overflow into next bucket */

    if (b->fetch_avail > 0) {
        /* Take partial chunk from current bucket */
        /* NB: this deals with (count == b->fetch_avail) case too */

        memcpy(s, &(b->fetch->data[b->blocksize - b->fetch_avail]),
               b->fetch_avail);
        s += b->fetch_avail;
        count -= b->fetch_avail;

        /* Set up next full bucket */
        b->fetch = b->fetch->next;
        b->fetch_avail = b->blocksize;
    }

    while (count >= b->blocksize) {
        /* Copy in full b->blocksize chunks */

        memcpy(s, b->fetch->data, b->blocksize);
        s += b->blocksize;
        count -= b->blocksize;

        /* Set up next full bucket */
        b->fetch = b->fetch->next;
        b->fetch_avail = b->blocksize;
    }

    /* Possible final (partial) bucket will be < b->blocksize */

    if (count > 0) {
        memcpy(s, b->fetch->data, count);

        /* More data to process in this bucket. b->fetch stays unchanged */
        b->fetch_avail = b->blocksize - count;
    }

    return (result);
}

/* buffer_fetch() *******************************************************
 *
 * Retrive block of data from buffer
 *      b:  Buffer
 *  offset: Offset into buffer
 *  count:  Number of characters to retrieve
 *   copy:  Generate separate copy of data.
 *          NIL => okay to return ptr to data in place if byte range
 *                 falls within a single msgblock object.
 ***********************************************************************/

void *buffer_fetch(struct buffer *b,
                   unsigned long offset, unsigned long count, BOOL copy)
{
    char *result;

    buffer_seek_offset(b, offset);

    if (count == 0)
        return (pool_strdup(b->pool, ""));

    if (copy || (b->fetch_avail < count + 1)) {
        result = pool_alloc(b->pool, count + 1);

        buffer_getblock(b, result, count);
        result[count] = '\0';
    } else {
        unsigned long init_offset = b->blocksize - b->fetch_avail;

        b->fetch->data[init_offset + count] = '\0';
        result = (char *) &(b->fetch->data[init_offset]);
    }
    return ((void *) result);
}

/* buffer_fetch_block() *************************************************
 *
 *
 * Fetch single block from buffer from current seek position. Repeated
 * calls will step through the buffer one block at a time.
 *      b: Buffer
 *   ptrp: Used to return next block
 *  sizep: Used to return size of next block
 *
 * Returns: T   => data available.
 *          NIL => no data available.
 ***********************************************************************/

BOOL
buffer_fetch_block(struct buffer *b,
                   unsigned char **ptrp, unsigned long *sizep)
{
    if (b->fetch == NIL)
        return (NIL);

    if (b->fetch->next) {
        *ptrp = &b->fetch->data[0];
        *sizep = b->blocksize;
        b->fetch = b->fetch->next;
    } else {
        *ptrp = &b->fetch->data[0];
        *sizep = b->blocksize - b->avail;
        b->fetch = NIL;
    }

    return (T);
}

/* ====================================================================== */

static void buffer_encode_common(struct buffer *b, char *t, char quote)
{
    unsigned char *s = (unsigned char *) t;
    static char hex[] = "0123456789abcdef";
    unsigned char c;

    while ((c=*s++)) {
        if (Uisalnum(c))
            bputc(b, c);
        else {
            bputc(b, quote);
            bputc(b, hex[c >> 4]);
            bputc(b, hex[c & 15]);
        }
    }
}

void buffer_encode_url(struct buffer *b, char *s)
{
    buffer_encode_common(b, s, '%');
}

void buffer_encode_canon(struct buffer *b, char *s)
{
    buffer_encode_common(b, s, '*');
}
