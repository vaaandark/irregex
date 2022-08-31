#ifndef RE_H_
#define RE_H_

#include "def.h"

extern void *RE_calloc(size_t nitems, size_t size);

extern void *RE_realloc(void *ptr, size_t nitems, size_t size);

#define alloc(size) RE_calloc(1, (size_t)size)

#define REGSTR_SIZE 128

typedef enum {
    CHARSET, META, UNKNOWN, END, BOUND
} TokenType;

typedef struct {
    TokenType t;
    union {
        bool ch[256];
        int bound[2];
        int metachar;
    } u;
} RE_Token;

typedef struct {
    char regstr[REGSTR_SIZE];
    char *str_read_pos;
    bool has_unget;
    RE_Token unget;
} RE_State;

typedef struct RE_Node RE_Node;

typedef struct {
    bool is_simple_atom;
    union {
        RE_Node *re;
        bool ch[256];
    } u;
} RE_Atom;

typedef struct {
    RE_Atom *a;
    int min, max; // bound
} RE_Piece;

#define BRANCH_INITIAL_SIZE 4

typedef struct {
    RE_Piece **p;
    int num;
    int size;
} RE_Branch;

#define RE_INITIAL_SIZE 8

typedef struct RE_Node {
    RE_Branch **b;
    int num;
    int size;
} RE_Node;

extern RE_State *RES_new(const char *str);

#define RES_drop(st) free(st)

extern void fill_by_range(int begin, int end, bool *ch, bool fill);
extern void Re_drop(RE_Node *n);

extern RE_Node *parse_regex(RE_State *st);

#ifdef DRAW_RELATIONSHIP
#define CHARSET_SHOW_MAX 16
extern void draw_relationship(FILE *f, RE_Node *re, char *regexp);
#endif

#endif
