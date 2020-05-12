/* Copytight (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#ifndef _CEZ_CORE_STRING
#define _CEZ_CORE_STRING
#pragma once

/* Some Basic utility functions */

char *string_itoa(struct pool *pool, unsigned long value);
char *string_itoa_tmp(unsigned long value);
BOOL string_isnumber(char *s);
BOOL ishex(char value);
unsigned long hex(char value);

BOOL string_atom_has_special(char *s);
char *string_atom_quote(struct pool *pool, char *s);

BOOL string_filename_valid(char *s);
char *string_prune(struct pool *pool, char *s, unsigned long maxlen);

char *string_trim_whitespace(char *string);

char *string_left(struct pool *p, char *string, unsigned long count);

BOOL string_isspace(char c);
BOOL string_iseol(char c);
char *string_next_token(char **sp);
char *string_get_token(char **sp);
BOOL string_skip_token(char **sp);

char *string_ustr(char *s, char *name);
char *string_get_value(char **sp);

char *string_next_line(char **sp);
char *string_get_line(char **sp);
char *string_get_lws_line(char **sp, BOOL squeeze);

char *string_canon_decode(char *string);
char *string_canon_encode(struct pool *pool, char *arg);

char *string_canon_path_encode(struct pool *pool, char *dir, char *file);
char *string_url_encode(struct pool *pool, char *arg);
char *string_url_decode(char *string);
char *string_url_decode_component(char **tp, char sep);

char *string_filename_encode(struct pool *pool, char *arg);

void string_malloc(void **ptr, unsigned long size);
void string_strdup(char **ptr, char *string);
void string_free(void **ptr);

char *string_ucase(char *s);
char *string_lcase(char *s);

BOOL string_has8bit(char *s0);
void string_strip8bit(char *s0);

void *
string_base64_decode(unsigned char *src,unsigned long srcl,unsigned long *len);

unsigned char *
string_qprint_decode(unsigned char *src,unsigned long srcl,
                     unsigned long *len);

char *string_expand(struct pool *pool, struct assoc *h, char *s);

char *
string_expand_crlf(struct pool *pool, char *s);

void
string_strip_crlf(char *s);

char *
string_email_split(struct pool *pool, char *s);

#endif

