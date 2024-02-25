#include "misc.h"

#include <string.h>
#include <stdlib.h>

int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

float frand()
{
    return rand() / (float)(RAND_MAX-1);
}
