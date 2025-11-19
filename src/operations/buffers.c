#include "kilo.h"

// #===Append Buffer===# 
void abAppend(struct appendbuff *ab, const char *s, int len)
{
    char *new = realloc(ab->b, ab->len + len); //Resize buffer

    if(new == NULL) return;
    memcpy(&new[ab->len], s, len); //Append new data
    ab->b = new;
    ab->len += len;
}

void abFree(struct appendbuff *ab)
{
    free(ab->b);
}

