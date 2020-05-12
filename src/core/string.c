/* Copyright (c) 2018-2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_core_pool.h"
#include "cez_core_assoc.h"
#include "cez_core_buffer.h"

#include "cez_core_string.h"

/* Some simple string (i.e: char *) manipulation routines */

/* string_itoa() ********************************************************
 *
 * Convert number into string using named pool as target
 *   pool: scratch pool
 *  value: value to convert
 *
 * Returns: text representation of number
 ***********************************************************************/

char *string_itoa(struct pool *pool, unsigned long value)
{
    unsigned long tmp, weight, nodigits;
    char *result, *s;

    /* All numbers contain at least one digit.
     * Find weight of most significant digit. */
    weight = 1;
    nodigits = 1;
    tmp = value / 10;
    while (tmp > 0) {
        weight *= 10;
        tmp /= 10;
        nodigits++;
    }

    result = pool_alloc(pool, nodigits + 1);

    for (s = result; weight > 0; weight /= 10) {
        if (value >= weight) {  /* Strictly speaking redundant... */
            *s++ = '0' + (value / weight);      /* Digit other than zero */
            value -= weight * (value / weight); /* Calculate remainder */
        } else
            *s++ = '0';
    }
    *s = '\0';

    return (result);
}

/* string_itoa_tmp() ****************************************************
 *
 * Convert number into string using (static) scratch buffer
 *  value: value to convert
 *
 * Returns: text representation of number.
 ***********************************************************************/

char *string_itoa_tmp(unsigned long value)
{
    static char result[64];     /* Larger than 64-bit int */
    char *s;
    unsigned long weight, tmp;

    /* All numbers contain at least one digit.
     * Find weight of most significant digit. */

    weight = 1;
    tmp = value / 10;
    while (tmp > 0) {
        weight *= 10;
        tmp /= 10;
    }

    for (s = result; weight > 0; weight /= 10) {
        if (value >= weight) {  /* Strictly speaking redundant... */
            *s++ = '0' + (value / weight);      /* Digit other than zero */
            value -= weight * (value / weight); /* Calculate remainder */
        } else
            *s++ = '0';
    }
    *s = '\0';

    return (result);
}

/* string_isnumber() *****************************************************
 *
 * Check that string is non-negative integer
 *************************************************************************/

BOOL
string_isnumber(char *s)
{
    if (!Uisdigit(*s))
        return(NIL);

    while (*s) {
        if (!Uisdigit(*s))
            return(NIL);
        s++;
    }
    return(T);
}

/* ====================================================================== */

/* ishex() **************************************************************
 *
 * Check for valid hexidecimal digit
 *  value: Character to check.
 *
 * Returns: T if character was valid hex digit.
 ***********************************************************************/

BOOL ishex(char value)
{
    if (Uisdigit(value))
        return (T);

    if ((value >= 'A') && (value <= 'F'))
        return (T);

    if ((value >= 'a') && (value <= 'f'))
        return (T);

    return (NIL);
}

/* hex() ****************************************************************
 *
 * Convert single hex digit into number.
 *  value: Hex digit to convert
 *
 * Returns: Number between 0 and 15
 ***********************************************************************/

unsigned long hex(char value)
{
    if (Uisdigit(value))
        return (value - '0');

    if ((value >= 'A') && (value <= 'F'))
        return (10 + value - 'A');

    if ((value >= 'a') && (value <= 'f'))
        return (10 + value - 'a');

    /* log_fatal("hex() called with illegal value"); */
    fprintf(stderr, "hex() called with illegal value");
    exit (1);
    return (0);                 /* NOTREACHED */
}

/* ====================================================================== */

/* RFC822 defines:
 *   specials    =  "(" / ")" / "<" / ">" / "@"  ; Must be in quoted-
 *                /  "," / ";" / ":" / "\" / <">  ;  string, to use
 *                /  "." / "[" / "]"              ;  within a word.
 */

/* string_atom_has_special() ********************************************
 *
 * Check if string contains significant RFC822 characters
 *   s: String to check
 *
 * Returns: T if strig contained significant characters.
 ***********************************************************************/

BOOL string_atom_has_special(char *s)
{
    char c;

    while ((c = *s++)) {
        switch (c) {
        case '(':
        case ')':
        case '<':
        case '>':
        case '@':
        case ',':
        case ';':
        case ':':
        case '\\':
        case '"':              /* NB: must use \" quoting if '"' used */
        case '.':
        case '[':
        case ']':
            return (T);
        }
    }
    return (NIL);
}

/* string_atom_quote() **************************************************
 *
 * Convert string to RFC822 quoted form.
 *  pool: Target pool.
 *     s: String to convert
 *
 * Returns: Quoted string
 ***********************************************************************/

char *string_atom_quote(struct pool *pool, char *s)
{
    char *result, *t;
    unsigned long count, len;

    /* Scan string for '"' chars, calculate length at the same time */
    count = 0;
    len = 0;
    for (t = s; *t; t++) {
        if (*t == '"')
            count++;
        len++;
    }

    if (count == 0)
        return (s);

    result = pool_alloc(pool, count + len + 1);

    t = result;
    while (*s) {
        if (*s == '"')
            *t++ = '\\';
        *t++ = *s++;
    }
    *t = '\0';

    return (result);
}

/* ====================================================================== */

/* string_filename_valid() ***********************************************
 *
 * Look for "/.." sequence in filenames
 *     s: String to check
 *
 * Returns: T if string contained invalid sequence
 ***********************************************************************/

BOOL string_filename_valid(char *s)
{
    /* Allow any sequence of characters: Cyrus will validate */
    /* Original concern was people playing silly buggers with Unix mailstore */
    return (T);
}

/* ====================================================================== */

/* string_prune() ********************************************************
 *
 * Generate pruned version of string if too long (replaces last three
 * characters with "...").
 *   pool: Target pool
 *      s: String to prune
 * maxlen: Maximum length of string before pruning applies.
 *
 * Returns: T if string contained invalid sequence
 ***********************************************************************/

char *string_prune(struct pool *pool, char *s, unsigned long maxlen)
{
    char *result;

    if (maxlen < (strlen("...") + 1))
        return (s);

    if (maxlen == 0)
        return (s);

    if (strlen(s) < maxlen)
        return (s);

    /* Make temporary copy of first maxlen chars of string */
    result = pool_alloc(pool, maxlen + 1);
    memcpy(result, s, maxlen);

    /* Replace last three characters with "..." */
    strcpy(result + maxlen - strlen("..."), "...");

    return (result);
}


/* ====================================================================== */

/* string_trim_whitespace() *********************************************
 *
 * Remove leading and trailing whitespace from a string
 *  string: String to prune
 *
 * Returns: Trimmed string.
 ***********************************************************************/

char *string_trim_whitespace(char *string)
{
    unsigned long len;

    /* Remove leading whitespace */
    while ((string[0] == ' ') ||
           (string[0] == '\t') ||
           (string[0] == '\015') || (string[0] == '\012'))
        string++;

    /* Remove traiing whitespace */
    len = strlen(string);
    while ((len > 0) &&
           ((string[len - 1] == ' ') ||
            (string[len - 1] == '\t') ||
            (string[len - 1] == '\015') || (string[len - 1] == '\012')))
        len--;

    /* Tie off the string */
    string[len] = '\0';

    return (string);
}

/* ====================================================================== */

/* String tokenisation routines */

/* string_isspace() *****************************************************
 *
 * Check if character is space character.
 ***********************************************************************/

BOOL string_isspace(char c)
{
    return ((c == ' ') || (c == '\t'));
}

/* string_isspace() *****************************************************
 *
 * Check if character is end of line
 ***********************************************************************/

BOOL string_iseol(char c)
{
    return ((c == '\0') || (c == '\015') || (c == '\012'));
}

/* string_next_token() **************************************************
 *
 * Find next token in string
 *    sp: Ptr to current string location (updated)
 *
 * Returns: Ptr to next token, NIL if none.
 ***********************************************************************/

char *string_next_token(char **sp)
{
    char *s = *sp;

    if (!s)
        return (NIL);

    while ((*s == ' ') || (*s == '\t')) /* Skip leading whitespace */
        s++;

    *sp = s;

    return ((*s) ? s : NIL);
}

/* string_get_token() ****************************************************
 *
 * Isolate next token in string
 *    sp: Ptr to current string location
 *        (updated to point to following token)
 *
 * Returns: Ptr to next token, NIL if none.
 ***********************************************************************/

char *string_get_token(char **sp)
{
    char *s = *sp, *result;

    if (!(s = string_next_token(sp)))
        return (NIL);

    /* Record position of this token */
    result = s;

    /* Find next whitespace character or end of string */
    while ((*s) && (*s != ' ') && (*s != '\t'))
        s++;

    /* Tie off the string unless \0 already reached */
    if (*s) {
        *s++ = '\0';

        while ((*s == ' ') || (*s == '\t'))
            s++;
    }

    /* Record position of first non-whitespace character for next caller */
    *sp = s;

    return (result);
}

/* string_skip_token() **************************************************
 *
 * Skip next token
 *    sp: Ptr to current string location
 *        (updated to point to following token)
 *
 * Returns: Ptr to next token, NIL if none.
 ***********************************************************************/

BOOL string_skip_token(char **sp)
{
    char *s;

    if (!(s = string_next_token(sp)))
        return (NIL);

    /* Find next whitespace character or end of string */
    while ((*s) && !string_isspace(*s))
        s++;

    /* Find next non-whitespace character */
    while ((*s == ' ') || (*s == '\t'))
        s++;

    /* Record position of first non-whitespace character for next caller */
    *sp = s;

    return (T);
}

/* ====================================================================== */

/* string_ustr() *********************************************************
 *
 * Case insensitive version of strstr()
 *
 ************************************************************************/

char *string_ustr(char *s, char *name)
{
    unsigned long len = strlen(name);

    while (*s && (strncasecmp(s, name, len) != 0))
        s++;

    return((*s) ? s : NIL);
}

/* string_get_value() ****************************************************
 *
 * Given \s*value\s+ or \s*"value" chop out the value.
 *
 ************************************************************************/

char *string_get_value(char **sp)
{
    char *s = *sp;
    char *value = NIL;

    while (isspace(*s))
        s++;

    if (*s == '"') {
        s++;
        value = s;
        while (*s && *s != '"')
            s++;
    } else {
        value = s;
        while (*s && *s != ' ' && *s != '\t')
            s++;
    }
    *s++ = '\0';

    *sp = s;
    return(value);
}

/* ====================================================================== */

/* string_next_line() ***************************************************
 *
 * Skip to next line
 *    sp: Ptr to current string location
 *        (updated to point to following line)
 *
 * Returns: Ptr to next line
 ***********************************************************************/

char *string_next_line(char **sp)
{
    char *s = *sp;
    char c;

    if (!(s && s[0]))
        return (NIL);

    while ((c = *s)) {
        if (c == '\015') {      /* CR */
            s++;
            if (*s == '\012')   /* CRLF */
                s++;
            break;
        } else if (c == '\012') {       /* LF */
            s++;
            break;
        }
        s++;
    }

    *sp = s;

    return (s);
}

/* string_get_line() *****************************************************
 *
 * Get a line
 *    sp: Ptr to current string location
 *        (updated to point to following line)
 *
 * Returns: Ptr to this line
 ***********************************************************************/

char *string_get_line(char **sp)
{
    char *s, c, *result;

    result = s = *sp;           /* Record position of this line */

    if (!(s && s[0]))
        return (NIL);

    while ((c = *s)) {
        if (c == '\015') {      /* CR */
            *s++ = '\0';
            if (*s == '\012')   /* CRLF */
                s++;
            break;
        } else if (c == '\012') {       /* LF */
            *s++ = '\0';
            break;
        }
        s++;
    }

    /* Record position of next line */
    *sp = s;

    return (result);
}

/* ====================================================================== */

/* string_get_lws_line() ************************************************
 *
 * Get a linear whitespace line (e.g: folded RFC822 or HTTP header line)
 *       sp: Ptr to current string location
 *           (updated to point to following line)
 *  squeeze: Remove superfluous spaces from result.
 *
 * Returns: Ptr to this line
 ***********************************************************************/

char *string_get_lws_line(char **sp, BOOL squeeze)
{
    char *s, *t, *result;
    BOOL quoted = NIL;

    s = *sp;

    if (!(s && s[0]))
        return (NIL);

    /* Skip leading whitespace */
    while ((*s == ' ') || (*s == '\t'))
        s++;

    /* Empty string before data proper starts? */
    if (s[0] == '\0') {
        *sp = s;
        return (s);
    }

    /* CR, LF or CRLF before data proper starts? */
    if ((s[0] == '\015') || (s[0] == '\012')) {
        result = s;
        if ((s[0] == '\015') && (s[1] == '\012')) {     /* CRLF */
            *s = '\0';
            s += 2;
        } else
            *s++ = '\0';        /* CR or LF */

        *sp = s;
        return (result);
    }

    result = t = s;             /* Record position of non-LWS */

    while (*s) {
        if ((*s == '\015') || (*s == '\012')) { /* CR, LF or CRLF */
            s += ((s[0] == '\015') && (s[1] == '\012')) ? 2 : 1;

            if ((*s != ' ') && (*s != '\t'))
                break;
            /* Is a continuation line */
            quoted = NIL;       /* Is "LWS" allowed? */
            if ((t > result) && (t[-1] != ' ')) {       /* Replace LWS with single SP */
                *t++ = ' ';
            }
        } else {
            if (*s == '"')
                quoted = (quoted) ? NIL : T;    /* Toggle quoted bit */

            if ((!quoted) && (squeeze) && ((*s == ' ') || (*s == '\t'))) {
                if ((t > result) && (t[-1] != ' '))
                    *t++ = ' ';
                s++;
            } else if (t < s)   /* Copy faithfully */
                *t++ = *s++;
            else {
                t++;
                s++;            /* Data hasn't moved yet */
            }
        }
    }

    /* Remove trailing whitespace. Easier to do this at the end */
    if (squeeze) {
        while ((t > result) && ((t[-1] == ' ') || (t[-1] == '\t')))
            t--;
    }
    *t = '\0';                  /* Tie off result string */

    *sp = s;                    /* Set up for next caller */

    return (result);
}

/* ====================================================================== */

/* Support routines for canon encoding */

static void canon_encode_char(unsigned char c, char **tp)
{
    unsigned char *t = (unsigned char *) *tp;
    static char hex[] = "0123456789abcdef";

    if (Uisalnum(c)) {
        *t++ = c;
    } else
        switch (c) {
        case '_':
        case '.':
        case '!':
        case '\'':
        case '(':
        case ')':
        case '-':
            *t++ = c;
            break;
        default:
            *t++ = '*';
            *t++ = hex[c >> 4];
            *t++ = hex[c & 15];
            break;
        }

    *tp = (char *) t;
}

static int canon_count_specials(char *s)
{
    unsigned long special_count = 0;
    char c;

    while ((c = *s++)) {
        if (!Uisalnum(c))
            switch (c) {
            case '_':
            case '.':
            case '!':
            case '\'':
            case '(':
            case ')':
            case '-':
                break;
            default:
                special_count++;
                break;
            }
    }
    return (special_count);
}

/* ====================================================================== */

/* string_canon_encode() ************************************************
 *
 * Encode string using canonical encoding for special characters
 *   pool: Target string
 *    arg: String to encode
 *
 * Returns: Encoded string
 ***********************************************************************/

char *string_canon_encode(struct pool *pool, char *arg)
{
    char *s, *result, *t;
    unsigned long specials;

    s = pool_strdup(pool, arg);

    /* Count special characters in string */
    if ((specials = canon_count_specials(s)) == 0)
        return (s);

    /* Need another copy, with space for encoded specials */
    result = pool_alloc(pool, strlen(s) + (2 * specials) + 1);

    /* Copy s to result, encoding specials */
    t = result;
    while (*s) {
        canon_encode_char((unsigned char) (*s), &t);
        s++;
    }
    *t = '\0';

    return (result);
}

/* string_canon_path_encode() *******************************************
 *
 * Encode (directory, file) combination using canonical encoding for
 * special characters.
 *   pool: Target string
 *    dir: Directory
 *   file: Filename
 *
 * Returns: Encoded string
 ***********************************************************************/

char *string_canon_path_encode(struct pool *pool, char *dir, char *file)
{
    char *s, *result, *t;
    unsigned long specials;

    if (file[0]) {
        if (dir && dir[0]) {
            s = pool_alloc(pool, strlen(dir) + 1 + strlen(file) + 1);
            strcpy(s, dir);
            t = s + strlen(s);
            *t++ = '/';
            strcpy(t, file);
        } else
            s = pool_strdup(pool, file);
    } else
        s = pool_strdup(pool, dir);

    /* Count special characters in string */
    if ((specials = canon_count_specials(s)) == 0)
        return (s);

    /* Need another copy, with space for encoded specials */
    result = pool_alloc(pool, strlen(s) + (2 * specials) + 1);

    /* Copy s to result, encoding specials */
    t = result;
    while (*s) {
        canon_encode_char(*s, &t);
        s++;
    }
    *t = '\0';

    return (result);
}

/* ====================================================================== */

/* string_canon_decode() ************************************************
 *
 * Decode string from canonical encoding (*XY hex-encoding for specials)
 * '*' choosen as a URL safe character: hopefully no browser will try
 * and URL enode this.
 *   string: Input string
 *
 * Returns: Decoded string, updating in place.
 ***********************************************************************/

char *string_canon_decode(char *string)
{
    char *s, *t;

    if (!string)
        return (NIL);

    /* Set all pointers to start of string */
    s = t = string;

    while (*t) {
        switch (*t) {
        case '*':
        case '%':
            if (ishex(t[1]) && ishex(t[2])) {   /* Better way to do this? */
                *s++ = (16 * hex(t[1])) + hex(t[2]);
                t += 3;
                continue;
            }
            /* Otherwise fall through to default behaviour */
        }

        if (s < t) {
            *s++ = *t++;        /* Copy string to new location */
        } else {
            s++;
            t++;                /* Just keep going */
        }
    }
    *s = '\0';                  /* Tie off string */

    return (string);
}

/* ====================================================================== */

/* Support routines for URL encoding */

static void url_encode_char(unsigned char c, char **tp)
{
    unsigned char *t = (unsigned char *) *tp;
    static char hex[] = "0123456789abcdef";

    if (Uisalnum(c)) {
        *t++ = c;
    } else
        switch (c) {
        case '_':
        case '.':
        case '!':
        case '*':
        case '\'':
        case '(':
        case ')':
        case '-':
            *t++ = c;
            break;
        default:
            *t++ = '%';
            *t++ = hex[c >> 4];
            *t++ = hex[c & 15];
            break;
        }

    *tp = (char *) t;
}

static int url_count_specials(char *s)
{
    unsigned long special_count = 0;
    char c;

    while ((c = *s++)) {
        if (!Uisalnum(c))
            switch (c) {
            case '_':
            case '.':
            case '!':
            case '*':
            case '\'':
            case '(':
            case ')':
            case '-':
                break;
            default:
                special_count++;
                break;
            }
    }
    return (special_count);
}

/* ====================================================================== */

/* string_url_encode() **************************************************
 *
 * Encode string using canonical encoding for special characters
 *   pool: Target string
 *    arg: String to encode
 *
 * Returns: Encoded string
 ***********************************************************************/

char *string_url_encode(struct pool *pool, char *arg)
{
    char *result, *t;
    unsigned long specials;

    /* Count special characters in string */
    if ((specials = url_count_specials(arg)) == 0)
        return (arg);

    /* Need another copy, with space for encoded specials */
    result = pool_alloc(pool, strlen(arg) + (2 * specials) + 1);

    /* Copy s to result, encoding specials */
    t = result;
    while (*arg) {
        url_encode_char(*arg, &t);
        arg++;
    }
    *t = '\0';

    return (result);
}

/* ====================================================================== */

/* string_url_decode() **************************************************
 *
 * Decode string from URL encoding (%XY hex-encoding for specials)
 *   string: Input string
 *
 * Returns: Decoded string, updating in place.
 ***********************************************************************/

char *string_url_decode(char *string)
{
    char *s, *t;

    if (!string)
        return (NIL);

    /* Set all pointers to start of string */
    s = t = string;

    while (*t) {
        switch (*t) {
        case '%':
            if (ishex(t[1]) && ishex(t[2])) {   /* Better way to do this? */
                *s++ = (16 * hex(t[1])) + hex(t[2]);
                t += 3;
                continue;
            }
            /* Otherwise fall through to default behaviour */
        }

        if (s < t) {
            *s++ = *t++;        /* Copy string to new location */
        } else {
            s++;
            t++;                /* Just keep going */
        }
    }
    *s = '\0';                  /* Tie off string */

    return (string);
}


/* ====================================================================== */

/* string_url_decode_component() ****************************************
 *
 * Decode single component of string from URL encoding (%XY hex-encoding
 * for specials)
 *      tp: Ptr to string (will be updated to point to next component)
 *     sep: Separator character (typically '/')
 * Returns: Decoded string, updating in place.
 ***********************************************************************/

char *string_url_decode_component(char **tp, char sep)
{
    char *s, *t, *result;

    /* Set all pointers to start of string */
    result = s = t = *tp;

    while ((*t) && (*t != sep)) {
        switch (*t) {
        case '+':              /* Is use of '+' documented anywhere? */
            *s++ = ' ';
            t++;
            continue;
        case '%':
            if (ishex(t[1]) && ishex(t[2])) {
                *s++ = (16 * hex(t[1])) + hex(t[2]);
                t += 3;
                continue;
            }
            /* Otherwise fall through to default behaviour */
        }

        if (s < t) {
            *s++ = *t++;        /* Copy string to new location */
        } else {
            s++;
            t++;                /* Just keep going */
        }
    }

    if (*t)                     /* Nuke '&' separator if it exists */
        *t++ = '\0';

    *s = '\0';                  /* Tie off string */

    *tp = t;                    /* Record location for next caller */

    return (result);
}

/* ====================================================================== */

/* string_filename_encode() *********************************************
 *
 * Mangle filename to URL friend form by replacing " " with "_". Only
 * needed because browsers appear to ignore filename parameters.
 *
 ***********************************************************************/

char *string_filename_encode(struct pool *pool, char *arg)
{
    char *result = pool_strdup(pool, arg);
    char *s = result;

    for (s = result; *s; s++) {
        if (*s == ' ')
            *s = '_';
    }

    return (result);
}

/* ====================================================================== */

/* Small malloc interface library */

/* string_malloc() ******************************************************
 *
 * Malloc space for string updating ptr argument
 *     ptr: Ptr to (void *) that will point to result.
 *    size: Number of bytes to allocated.
 ***********************************************************************/

void string_malloc(void **ptr, unsigned long size)
{
    if (*ptr)
        free(*ptr);

    if ((*ptr = malloc(size)))
        return;

    /* log_fatal("Out of memory!"); */
    fprintf(stderr, "Out of memory!");
    exit (1);
    /* NOTREACHED */
    abort();
}

/* string_strdup() ******************************************************
 *
 * Strdup string:
 *     ptr: Ptr to (void *) that will point to result.
 *  string: String to strdup
 ***********************************************************************/

void string_strdup(char **ptr, char *string)
{
    if (*ptr)
        free(*ptr);

    if (string == NIL) {
        *ptr = NIL;
        return;
    }

    if ((*ptr = strdup(string)))
        return;

    /* log_fatal("Out of memory!"); */
    fprintf(stderr, "Out of memory!");
    exit (1);
    /* NOTREACHED */
    abort();
}

/* string_free() ********************************************************
 *
 * Free string, clearing ptr in process.
 *     ptr: Ptr to (void *) that will be cleared after free
 ***********************************************************************/

void string_free(void **ptr)
{
    free(*ptr);
    *ptr = NIL;
}

/* string_ucase() *******************************************************
 *
 * Wrapper function to keep compiler happy
 * 
 ***********************************************************************/

char *string_ucase (char *s)
{
  unsigned char *t;
				/* if lowercase covert to upper */
  for (t = (unsigned char *)s; *t; t++) {
      if (!(*t & 0x80) && islower (*t))
          *t = toupper (*t);
  }
  return s;			/* return string */
}


/* string_lcase() *******************************************************
 *
 * Wrapper function to keep compiler happy
 * 
 ***********************************************************************/

char *string_lcase (char *s)
{
  unsigned char *t;
				/* if uppercase covert to lower */
  for (t = (unsigned char *)s; *t; t++) {
      if (!(*t & 0x80) && isupper (*t))
          *t = tolower (*t);
  }
  return s;			/* return string */
}

/* ====================================================================== */

/* string_has8bit() *******************************************************
 *
 * Does string contain 8 bit characters.
 *
 **************************************************************************/

BOOL string_has8bit(char *s0)
{
    unsigned char *s = (unsigned char *)s0;

    if (!s)
        return(NIL);

    while (*s) {
        if ((*s) & 0x80)
            return(T);
        s++;
    }
    return(NIL);
}

/* string_strip8bit() *****************************************************
 *
 * Strip all 8bit characters from string in place. Can cope with NIL input.
 *
 **************************************************************************/

void string_strip8bit(char *s0)
{
    unsigned char *s = (unsigned char *)s0;
    unsigned char *t;

    if (!s)
        return;

    while (*s && (((*s) & 0x80) == 0))
        s++;

    if (*s == '\0')
        return;

    /* Found an 8bit character */
    t = s;
    while (*s) {
        while (*s && ((*s) & 0x80))
            s++;

        while (*s && (((*s) & 0x80) == 0))
            *t++ = *s++;
        
    }
    if (*t != '\0')
        *t = '\0';
}

/* ====================================================================== */

/* Couple of utility routines stolen from c-client, just so that we
 * don't have to link the prayer frontend processes against c-client.
 *
 * These versions update in place rather allocating memory.
 */

static unsigned char myhex2byte(unsigned char c1,unsigned char c2)
{
				/* merge the two nybbles */
  return ((c1 -= (isdigit (c1) ? '0' : ((c1 <= 'Z') ? 'A' : 'a') - 10)) << 4) +
    (c2 - (isdigit (c2) ? '0' : ((c2 <= 'Z') ? 'A' : 'a') - 10));
}

/* Convert BASE64 contents to binary
 * Accepts: source
 *	    length of source
 *	    pointer to return destination length
 * Returns: destination as binary or NIL if error
 */

#define WSP 0176		/* NUL, TAB, LF, FF, CR, SPC */
#define JNK 0177
#define PAD 0100

void *
string_base64_decode(unsigned char *src,unsigned long srcl,unsigned long *len)
{
  char c;
  void *ret = src;
  char *d = (char *) ret;
  int e;
  static char decode[256] = {
   WSP,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,WSP,WSP,JNK,WSP,WSP,JNK,JNK,
   JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
   WSP,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,076,JNK,JNK,JNK,077,
   064,065,066,067,070,071,072,073,074,075,JNK,JNK,JNK,PAD,JNK,JNK,
   JNK,000,001,002,003,004,005,006,007,010,011,012,013,014,015,016,
   017,020,021,022,023,024,025,026,027,030,031,JNK,JNK,JNK,JNK,JNK,
   JNK,032,033,034,035,036,037,040,041,042,043,044,045,046,047,050,
   051,052,053,054,055,056,057,060,061,062,063,JNK,JNK,JNK,JNK,JNK,
   JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
   JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
   JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
   JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
   JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
   JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
   JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
   JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK
  };
  *len = 0;			/* in case we return an error */

				/* simple-minded decode */
  for (e = 0; srcl--; ) switch (c = decode[*src++]) {
  default:			/* valid BASE64 data character */
    switch (e++) {		/* install based on quantum position */
    case 0:
      *d = c << 2;		/* byte 1: high 6 bits */
      break;
    case 1:
      *d++ |= c >> 4;		/* byte 1: low 2 bits */
      *d = c << 4;		/* byte 2: high 4 bits */
      break;
    case 2:
      *d++ |= c >> 2;		/* byte 2: low 4 bits */
      *d = c << 6;		/* byte 3: high 2 bits */
      break;
    case 3:
      *d++ |= c;		/* byte 3: low 6 bits */
      e = 0;			/* reinitialize mechanism */
      break;
    }
    break;
  case WSP:			/* whitespace */
    break;
  case PAD:			/* padding */
    switch (e++) {		/* check quantum position */
    case 3:			/* one = is good enough in quantum 3 */
				/* make sure no data characters in remainder */
      for (; srcl; --srcl) switch (decode[*src++]) {
				/* ignore space, junk and extraneous padding */
      case WSP: case JNK: case PAD:
	break;
      default:			/* valid BASE64 data character */
	/* This indicates bad MIME.  One way that it can be caused is if
	   a single-section message was BASE64 encoded and then something
	   (e.g. a mailing list processor) appended text.  The problem is
	   that in 1 out of 3 cases, there is no padding and hence no way
	   to detect the end of the data.  Consequently, prudent software
	   will always encapsulate a BASE64 segment inside a MULTIPART.
	   */
          srcl = 1;		/* Bail out */
	break;
      }
      break;
    case 2:			/* expect a second = in quantum 2 */
      if (srcl && (*src == '=')) break;
    default:			/* impossible quantum position */
      return NIL;
    }
    break;
  case JNK:			/* junk character */
    return NIL;
  }
  *len = d - (char *) ret;	/* calculate data length */
  *d = '\0';			/* NUL terminate just in case */
  return ret;			/* return the string */
}

/* Convert QUOTED-PRINTABLE contents to 8BIT
 * Accepts: source
 *	    length of source
 * 	    pointer to return destination length
 * Returns: destination as 8-bit text or NIL if error
 */

unsigned char *
string_qprint_decode(unsigned char *src,unsigned long srcl,
                     unsigned long *len)
{
  unsigned char *ret = src;
  unsigned char *d = ret;
  unsigned char *t = d;
  unsigned char *s = src;
  unsigned char c,e;
  *len = 0;			/* in case we return an error */
				/* until run out of characters */
  while (((unsigned long) (s - src)) < srcl) {
    switch (c = *s++) {		/* what type of character is it? */
    case '=':			/* quoting character */
      if (((unsigned long) (s - src)) < srcl) switch (c = *s++) {
      case '\0':		/* end of data */
	s--;			/* back up pointer */
	break;
      case '\015':		/* non-significant line break */
	if ((((unsigned long) (s - src)) < srcl) && (*s == '\012')) s++;
      case '\012':		/* bare LF */
	t = d;			/* accept any leading spaces */
	break;
      default:			/* two hex digits then */
	if (!(isxdigit (c) && (((unsigned long) (s - src)) < srcl) &&
	      (e = *s++) && isxdigit (e))) {
	  /* This indicates bad MIME.  One way that it can be caused is if
	     a single-section message was QUOTED-PRINTABLE encoded and then
	     something (e.g. a mailing list processor) appended text.  The
	     problem is that there is no way to determine where the encoded
	     data ended and the appended crud began.  Consequently, prudent
	     software will always encapsulate a QUOTED-PRINTABLE segment
	     inside a MULTIPART.
	   */
	  *d++ = '=';		/* treat = as ordinary character */
	  *d++ = c;		/* and the character following */
	  t = d;		/* note point of non-space */
	  break;
	}
	*d++ = myhex2byte (c,e);/* merge the two hex digits */
	t = d;			/* note point of non-space */
	break;
      }
      break;
    case ' ':			/* space, possibly bogus */
      *d++ = c;			/* stash the space but don't update s */
      break;
    case '\015':		/* end of line */
    case '\012':		/* bare LF */
      d = t;			/* slide back to last non-space, drop in */
    default:
      *d++ = c;			/* stash the character */
      t = d;			/* note point of non-space */
    }      
  }
  *d = '\0';			/* tie off results */
  *len = d - ret;		/* calculate length */
  return ret;			/* return the string */
}

/* ====================================================================== */

/* string_expand() *******************************************************
 *
 * Expand $name and ${name} references in string using hash for lookups
 ************************************************************************/

char *string_expand(struct pool *pool, struct assoc *h, char *s)
{
    struct buffer *b = buffer_create(pool, 48); /* Typically short */
    char *t, *value;

    while (*s) {
        if (*s == '$') {
            if (s[1] == '{') {
                /* Find end of ${name} expansion */
                if (!(t = strchr(s, '}')))
                    return (NIL);

                s += 2;
                *t++ = '\0';
            } else {
                /* Find end of $name expansion (w/s ends token) */
                t = ++s;
                while (Uisalpha(*t))
                    t++;

                if (*t)
                    *t++ = '\0';
            }

            /* Attempt to expand value. Name not found => expansion fails */
            if (!(value = assoc_lookup(h, s)))
                return (NIL);
            bputs(b, value);
            s = t;
        } else if (*s == '\\') {
            /* Quoted character */
            s++;
            if (*s == '\0')
                return (NIL);
            switch (*s) {
            case 'n':
                bputc(b, '\n');
                break;
            case 'r':
                bputc(b, '\r');
                break;
            case 't':
                bputc(b, '\t');
                break;
            default:
                bputc(b, *s);
                break;
            }
            s++;
        } else {
            /* Simple character */
            bputc(b, *s++);
        }
    }
    return (buffer_fetch(b, 0, buffer_size(b), NIL));
}

/* ====================================================================== */

/* string_expand_crlf() ***************************************************
 *
 * Convert all CRLF, CR, LF sequences in a string into CRLF.
 ************************************************************************/

char *
string_expand_crlf(struct pool *pool, char *s)
{
    struct buffer *b = buffer_create(pool, 64);

    while (*s) {
        if ((s[0] == '\015') && (s[1] == '\012')) {
            bputs(b, ""CRLF);
            s += 2;
        } else if ((s[0] == '\015') || (s[0] == '\012')) {
            bputs(b, ""CRLF);
            s++;
        } else {
            bputc(b, *s);
            s++;
        }
    }
    return(buffer_fetch(b, 0, buffer_size(b), NIL));
}

/* string_string_crlf() **************************************************
 *
 * Replace all CRLF, CR, LF sequences in message with simple LF. Only
 * suitable if target platform is Unix: better to ship as is, use
 * string_expand_crlf on the way back for consistency.
 ************************************************************************/

void
string_strip_crlf(char *s)
{
    char *d = s;

    while (*s) {
        if ((s[0] == '\015') && (s[1] == '\012')) {
            *d++ = '\n';
            s += 2;
        } else if (d < s) {
            *d++ = *s++;
        } else {
            s++; d++;
        }
    }
    *d = '\0';
}

/* ====================================================================== */

/* Render "a,b,c" as "a, b, c" so the browser can wrap */

static int
string_email_split_len(char *s)
{
    int len = 0;

    while (*s) {
        len += (s[0] == ',' && !string_isspace(s[1])) ? 2 : 1;
        s++;
    }

    return(len);
}

char *
string_email_split(struct pool *pool, char *s)
{
    char *result = pool_alloc(pool, string_email_split_len(s)+1);
    char *t = result;

    while (*s) {
        if (s[0] == ',' && !string_isspace(s[1])) {
            *t++ = *s++;
            *t++ = ' ';
        } else {
            *t++ = *s++;
        }
    }
    *t = '\0';
    return(result);
}

