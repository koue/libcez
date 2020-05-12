
#include "cez_core_pool.h"

void *
xmalloc(unsigned long size)
{
    void *result = malloc(size);

    if (result)
        return(result);

    fprintf(stderr, "Failed to allocate %lu bytes", size);
    exit(1);
}

void *
xrealloc(void *old, unsigned long size)
{
    void *result = realloc(old, size);

    if (result)
        return(result);

    fprintf(stderr, "Failed to allocate %lu bytes", size);
    exit(1);
}

