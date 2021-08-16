#include "ansi_esc.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void
ansi_set(int n, ...)
{
    int i;
    va_list args;
    printf(ANSI_ESC);
    va_start(args, n);

    for (i = 0; i < n; i++) {
        if (i > 0)
            printf(";");
        const char * s = va_arg(args, const char *);
        printf("%s", s);
    }
    printf("m");
    va_end(args);
}

void
ansi_reset()
{
    printf(ANSI_RESET);
}
