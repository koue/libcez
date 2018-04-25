/* Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

int main(int argc, char *argv[])
{
    struct template *template;
    struct pool *pool = pool_create(4096);
    char *prefix;
    char *target;
    char *source;
    FILE *file;
    char *err;

    if (argc != 4) {
        fprintf(stderr, "Usage: prefix target template\n");
        exit(1);
    }
    prefix = argv[1];
    target = argv[2];
    source = argv[3];

    if (!(template = template_parse(NIL, NIL, source, pool))) {
        fprintf(stderr, "Unable to read template file %s\n", source);
        exit(1);
    }

    if ((err = str_fetch(template->error)) && err[0]) {
        fputs(err, stderr);
        exit(1);
    }

    if ((file=fopen(target, "w")) == NULL) {
        fprintf(stderr, "Failed to open %s: %s\n", target, strerror(errno));
        exit(1);
    }
    template_compile(prefix, template, file);
    fclose(file);

    return(0);
}
