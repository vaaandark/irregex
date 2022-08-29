#ifndef DEF_H_
#define DEF_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define DRAW_NFA

#define DRAW_RELATIONSHIP

#define unimplemented(msg) \
    do { \
        fprintf(stderr, "\033[1;033mregex:\033[0m \033[1;31mmunimplemented\033[0m @%s(%d) : %s\n", __FILE__, __LINE__, msg); \
        abort(); \
    } while (0)

#define panic(msg) \
    do { \
        fprintf(stderr, "\033[1;31mpanic\033[0m @%s(%d) : %s\n", __FILE__, __LINE__, msg); \
        abort(); \
    } while (0)

#endif
