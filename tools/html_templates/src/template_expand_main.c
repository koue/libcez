/* Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

static BOOL
read_var_file(struct pool *pool, struct assoc *h, char *name)
{
    char *data;
    char **lines;
    char *s;
    char *key;

    if (!(data = template_parse_read_file(name, pool))) {
        fprintf(stderr, "Var file %s not found\b", name);
        return(NIL);
    }

    lines  = template_parse_split_lines(data, pool);
    while (*lines) {
        s = *lines;

        while (*s == ' ' || *s == '\t')
            s++;

        if (!s[0] || s[0] == '#') {
            lines++;
            continue;
        }

        key  = template_getvar(&s, NIL, NIL);
        if (!key) {
            fprintf(stderr, "Invalid line in key file: %s\n", *lines);
            lines++;
            continue;
        }
        while ((*s == ' ') || (*s == '\t'))
            s++;

        if (!(s && s[0]))
            s = "1";

        if (!(key && key[0] && (key[0] != '#'))) {
            lines++;
            continue;
        }

        assoc_update(h, key, s, T);   /* T: strdup key and value */

        if (key[0] == '@') {
            char *s = strrchr(key, '-');

            *s = '\0';
            assoc_update(h, key, "1", T);
            *s = '-';
        }
        lines++;
    }
    return(T);
}

int main(int argc, char *argv[])
{
    struct template_vals *tvals;
    struct template_vals_urlstate *urlstate;
    struct pool *pool = pool_create(4096);
    struct buffer *b = buffer_create(pool, 1024);
    char *target;
    char *source;
    int i, c;
    FILE *file;

    if (argc < 4) {
        fprintf(stderr, "Usage: target template vars [vars...]\n");
        exit(1);
    }
    target = argv[1];
    source = argv[2];

    urlstate = template_vals_urlstate_create(pool);
    urlstate->url_prefix_icons = "../../files/icons";
    urlstate->url_prefix_bsession = "";
    urlstate->sequence = 1;
    urlstate->use_short = NIL;
    urlstate->test_mode = T;

    tvals = template_vals_create(pool, NIL, NIL, NIL, NIL, urlstate);

    for (i = 3; i < argc; i++)
        read_var_file(pool, tvals->vals, argv[i]);


    if (!template_expand(source, tvals, b)) {
        fputs(str_fetch(tvals->error), stderr);
        exit(1);
    }

    if ((file=fopen(target, "w")) == NULL) {
        fprintf(stderr, "Failed to open %s: %s\n", target, strerror(errno));
        exit(1);
    }

    buffer_rewind(b);
    while ((c=bgetc(b)) != EOF)
        fputc(c, file);

    fclose(file);

    return(0);
}
