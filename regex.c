#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define DRAW_NFA
//#undef DRAW_NFA

#define DRAW_RELATIONSHIP
//#undef DRAW_RELATIONSHIP

#define unimplemented(msg) \
    do { \
        fprintf(stderr, "unimplemented@%d: %s\n", __LINE__, msg); \
        abort(); \
    } while (0)

#define panic(msg) \
    do { \
        fprintf(stderr, "panic@%d: %s\n", __LINE__, msg); \
        abort(); \
    } while (0)

static void *RE_calloc(size_t nitems, size_t size) {
    void *mem = calloc(nitems, size);
    if (!mem) {
        panic("memory alloction error");
    }
    return mem;
}

static void *RE_realloc(void *ptr, size_t nitems, size_t size) {
    void *mem = realloc(ptr, nitems * size);
    if (!mem) {
        panic("memory alloction error");
    }
    return mem;
}

#define alloc(size) RE_calloc(1, (size_t)size)

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

#define REGSTR_SIZE 128

typedef struct {
    char regstr[REGSTR_SIZE];
    char *str_read_pos;
    RE_Token *unget;
} RE_State;

static RE_State *RES_init(char *str) {
    RE_State *st = (RE_State *)alloc(sizeof(RE_State));
    if (!st) {
        panic("RE_State is empty");
    }
    strcpy(st->regstr, str);
    st->str_read_pos = str;
    st->unget = NULL;
    return st;
}

#define meta_chars "^$*+?{}()|"

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

static RE_Token *Token_new(void) {
    RE_Token *token = (RE_Token *)alloc(sizeof(RE_Token));
    token->t = UNKNOWN;
    return token;
}

static RE_Branch *Branch_new(void) {
    RE_Branch *branch = (RE_Branch *)alloc(sizeof(RE_Branch));
    branch->num = 0;
    branch->size = BRANCH_INITIAL_SIZE;
    branch->p = (RE_Piece **)RE_calloc(branch->size, sizeof(RE_Piece *));
    return branch;
}

static RE_Branch *Branch_resize(RE_Branch *branch) {
    branch->size = branch->size * 2 + 4;
    branch->p = (RE_Piece **)RE_realloc(branch->p, branch->size, sizeof(RE_Piece *));
    return branch;
}

static RE_Node *Node_new(void) {
    RE_Node *node = (RE_Node *)alloc(sizeof(RE_Node));
    node->num = 0;
    node->size = RE_INITIAL_SIZE;
    node->b = (RE_Branch **)RE_calloc(node->size, sizeof(RE_Branch *));
    return node;
}

static RE_Node *Node_resize(RE_Node *node) {
    node->size = node->size * 2 + 4;
    node->b = (RE_Branch **)RE_realloc(node->b, node->size, sizeof(RE_Branch *));
    return node;
}

static void fill_by_range(int begin, int end, bool *ch, bool fill) {
    for (int i = begin; i <= end; ++i) {
        ch[i] = fill;
    }
}

static void fill_by_string(char *s, bool *ch, bool fill) {
    while (*s) {
        ch[(int)*s] = fill;
        s++;
    }
}

static void fill_by_char(int c, bool *ch, bool fill) {
    ch[c] = fill;
}

static void Token_display(const RE_Token *self) {
    if (!self) {
        panic("token is empty");
    }
    bool is_meta = (self->t == META);
    if (!is_meta) {
        putchar('[');
    }
    for (int i = 0; i < 256; ++i) {
        if (self->u.ch[i]) {
            putchar(i);
        }
    }
    if (!is_meta) {
        putchar(']');
    }
}

static void print_tokens(RE_Token **tokens) {
    RE_Token **pt = tokens;
    while ((*pt)->t != END) {
        Token_display(*pt);
        putchar(' ');
        pt++;
    }
    putchar('\n');
}

static RE_Token *get_token_escaped(RE_State *st) {
    RE_Token *res = Token_new();

    st->str_read_pos++;
    res->t = CHARSET;
    char c = *st->str_read_pos;
    switch (c) {
    case '\0':
        panic("regex expression should not end with '\\'");
        break;
    case 'd':
        fill_by_range('0', '9', res->u.ch, true);
        break;
    case 'D':
        fill_by_range(1, 255, res->u.ch, true);
        fill_by_range('0', '9', res->u.ch, false);
        break;
    case 'f':
        fill_by_char('\x0c', res->u.ch, true);
        break;
    case 'n':
        fill_by_char('\x0a', res->u.ch, true);
        break;
    case 'r':
        fill_by_char('\x0d', res->u.ch, true);
        break;
    case 's':
        fill_by_string(" \f\n\r\t\v", res->u.ch, true);
        break;
    case 'S':
        fill_by_range(1, 255, res->u.ch, true);
        fill_by_string(" \f\n\r\t\v", res->u.ch, false);
        break;
    case 't':
        fill_by_char('\x09', res->u.ch, true);
        break;
    case 'v':
        fill_by_char('\x0b', res->u.ch, true);
        break;
    case 'w':
        fill_by_range('a', 'z', res->u.ch, true);
        fill_by_range('A', 'Z', res->u.ch, true);
        fill_by_range('0', '9', res->u.ch, true);
        fill_by_char('-', res->u.ch, true);
        break;
    case 'W':
        fill_by_range(1, 255, res->u.ch, true);
        fill_by_range('a', 'z', res->u.ch, false);
        fill_by_range('A', 'Z', res->u.ch, false);
        fill_by_range('0', '9', res->u.ch, false);
        fill_by_char('-', res->u.ch, false);
        break;
    case 'x':
        if (st->str_read_pos[1] == '\0' || st->str_read_pos[2] == '\0') {
            panic("'\\xnn' needs two more xdigits");
        }
        if (isxdigit(st->str_read_pos[1]) && isxdigit(st->str_read_pos[2])) {
            int xd;
            sscanf(st->str_read_pos + 1, "%x", &xd);
            fill_by_char(xd, res->u.ch, true);
            st->str_read_pos += 2;
        } else {
            panic("'\\xnn' needs two xdigits");
        }
        break;
    default:
        res->u.ch[(int)*st->str_read_pos] = true;
        break;
    }
    st->str_read_pos++;

    return res;
}

#define cmp_class(s, class_name, shift) \
    (!strncmp(s, class_name, strlen(class_name)) && (shift = strlen(class_name)) > 0)

// charset is complex and I am lazy
static RE_Token *get_token_charset(RE_State *st) {
    RE_Token *res = Token_new();
    res->t = CHARSET;

    bool fill = true;

    st->str_read_pos++;
    // for the first character in the bracket
    char first = *st->str_read_pos;
    if (first == ']' || first == '-') {
        res->u.ch[(int)first] = fill;
        st->str_read_pos++;
    } else if (first == '^') {
        fill_by_range(1, 255, res->u.ch, fill);
        fill = false;
        first = *(++st->str_read_pos);
        if (first == ']' || first == '-') {
            res->u.ch[(int)first] = fill;
            st->str_read_pos++;
        }
    }

    while (*st->str_read_pos != ']') {
//    NFANode_drop(&b->begin);
        switch (*st->str_read_pos) {
        case '\0':
            panic("bracket should end with ']'");
            break;
        case '-':
            if (st->str_read_pos[1] != ']') {
                fill_by_range(st->str_read_pos[-1], st->str_read_pos[1], \
                        res->u.ch, fill);
                st->str_read_pos++;
            } else { // ']' can be the last character in bracket
                res->u.ch[(int)*st->str_read_pos] = fill;
            }
            st->str_read_pos++;
            break;
        case '[':
            if (st->str_read_pos[1] != ':') {
                goto not_special;
            }
            // character class
            // support: ascii, alnum, alpha, blank, cntrl, digit, graph, lower,
            // print, punct, space, upper, word, xdigit
            st->str_read_pos += 2;
            int begin, end;
            int shift = 0;
            if (cmp_class(st->str_read_pos, "ascii", shift)) {
                fill_by_range(0, 255, res->u.ch, fill);
            } else if (cmp_class(st->str_read_pos, "alnum", shift)) {
                fill_by_range((int)'a', (int)'z', res->u.ch, fill);
                fill_by_range((int)'A', (int)'Z', res->u.ch, fill);
                fill_by_range((int)'0', (int)'9', res->u.ch, fill);
            } else if (cmp_class(st->str_read_pos, "alpha", shift)) {
                fill_by_range((int)'a', (int)'z', res->u.ch, fill);
                fill_by_range((int)'A', (int)'Z', res->u.ch, fill);
            } else if (cmp_class(st->str_read_pos, "blank", shift)) {
                res->u.ch[(int)' '] = fill;
                res->u.ch[(int)'\t'] = fill;
            } else if (cmp_class(st->str_read_pos, "cntrl", shift)) {
                fill_by_range((int)'\x00', (int)'\x1F', res->u.ch, fill);
                res->u.ch[(int)'\x7F'] = fill;
            } else if (cmp_class(st->str_read_pos, "digit", shift)) {
                fill_by_range((int)'0', (int)'9', res->u.ch, fill);
            } else if (cmp_class(st->str_read_pos, "graph", shift)) {
                fill_by_range((int)'\x21', (int)'\x7E', res->u.ch, fill);
            } else if (cmp_class(st->str_read_pos, "lower", shift)) {
                fill_by_range((int)'a', (int)'z', res->u.ch, fill);
            } else if (cmp_class(st->str_read_pos, "print", shift)) {
                fill_by_range((int)'\x20', (int)'\x7E', res->u.ch, fill);
            } else if (cmp_class(st->str_read_pos, "punct", shift)) {
                fill_by_string("][!\"#$%&'()*+,./:;<=>?@\\^_`{|}~-", res->u.ch, \
                        fill);
            } else if (cmp_class(st->str_read_pos, "space", shift)) {
                fill_by_string(" \t\r\n\v\f", res->u.ch, fill);
            } else if (cmp_class(st->str_read_pos, "upper", shift)) {
                fill_by_range((int)'A', (int)'Z', res->u.ch, fill);
            } else if (cmp_class(st->str_read_pos, "word", shift)) {
                fill_by_range((int)'a', (int)'z', res->u.ch, fill);
                fill_by_range((int)'A', (int)'Z', res->u.ch, fill);
                fill_by_range((int)'0', (int)'9', res->u.ch, fill);
                res->u.ch[(int)'-'] = fill;
            } else if (cmp_class(st->str_read_pos, "xdigit", shift)) {
                fill_by_range((int)'a', (int)'f', res->u.ch, fill);
                fill_by_range((int)'A', (int)'F', res->u.ch, fill);
                fill_by_range((int)'0', (int)'9', res->u.ch, fill);
            } else {
                panic("invalid character class name");
            }
            st->str_read_pos += shift;
            if (strncmp(st->str_read_pos, ":]", 2) != 0) {
                panic("character class should be ended with \":\"");
            }
            st->str_read_pos += 2;
            break;
        default:
not_special:
            res->u.ch[(int)*st->str_read_pos] = fill;
            st->str_read_pos++;
            break;
        }
    }

    st->str_read_pos++;

    return res;
}

static RE_Token *get_token_bound(RE_State *st) {
    RE_Token *res = Token_new();
    res->t = BOUND;

    enum {
        S_START, S_READLEFT, S_READLEFT_BEGIN, S_READRIGHT, S_END
    } state = S_START;

    while (state != S_END) {
        switch (state) {
        case S_START:
            if (*st->str_read_pos == '{') {
                res->u.bound[0] = 0;
                st->str_read_pos++;
                state = S_READLEFT_BEGIN;
            } else {
                panic("illegal bound");
            }
            break;

        case S_READLEFT_BEGIN:
            if (isdigit(*st->str_read_pos)) {
                res->u.bound[0] = *st->str_read_pos - '0';
                st->str_read_pos++;
                state = S_READLEFT;
            } else {
                panic("illegal bound");
            }
            break;

        case S_READLEFT:
            if (*st->str_read_pos == ',') {
                res->u.bound[1] = -1;
                st->str_read_pos++;
                state = S_READRIGHT;
            } else if (*st->str_read_pos == '}') {
                res->u.bound[1] = res->u.bound[0];
                st->str_read_pos++;
                state = S_END;
            } else if (isdigit(*st->str_read_pos)) {
                res->u.bound[0] *= 10;
                res->u.bound[0] += *st->str_read_pos - '0';
                st->str_read_pos++;
                state = S_READLEFT;
            } else {
                panic("illegal bound");
            }
            break;

        case S_READRIGHT:
            if (*st->str_read_pos == '}') {
                st->str_read_pos++;
                state = S_END;
            } else if (isdigit(*st->str_read_pos)) {
                if (res->u.bound[1] == -1) {
                    res->u.bound[1] = *st->str_read_pos - '0';
                } else {
                    res->u.bound[1] *= 10;
                    res->u.bound[1] += *st->str_read_pos - '0';
                }
                st->str_read_pos++;
                state = S_READRIGHT;
            } else {
                panic("illegal bound");
            }
            break;
        default:
            panic("illegal bound");
        }
    }

    if (res->u.bound[1] >= 0 && res->u.bound[0] > res->u.bound[1]) {
        panic("illegal bound");
    }

    return res;
}

static RE_Token *get_token(RE_State *st) {
    RE_Token *res = NULL;

    if (st->unget) { // has unget token
        res = st->unget;
        st->unget = NULL;
    } else {
        char c = *st->str_read_pos;
        if (c == '[') {
            res = get_token_charset(st);
        } else if (c == '\\') {
            res = get_token_escaped(st);
        } else if (c == '{') {
            res = get_token_bound(st);
        } else {
            res = Token_new();
            if (c == '\0') {
                res->t = END;
            } else if (strchr(meta_chars, c)) {
                res->t = META;
                res->u.metachar = c;
            } else if (c == '.') {
                res->t = CHARSET;
                fill_by_range(1, 255, res->u.ch, true);
            } else {
                res->t = CHARSET;
                fill_by_char(c, res->u.ch, true);
            }
            st->str_read_pos++;
        }
    }

    return res;
}

#define unget_token(st, t) \
    st->unget = t

static RE_Atom *parse_atom(RE_State *st) {
    RE_Atom *res = alloc(sizeof(RE_Atom));

    extern RE_Node *parse_regex(RE_State *st);

    RE_Token *peek = get_token(st);

    if (peek->t == META) {
        if (peek->u.metachar == '(') {
            res->is_simple_atom = false;
            res->u.re = parse_regex(st);
        } else {
            panic("illegal regex");
        }
    } else if (peek->t == CHARSET) {
        res->is_simple_atom = true;
        memcpy(&res->u, &peek->u, sizeof(res->u));
    } else {
        panic("illegal regex");
    }
    return res;
}

static RE_Piece *parse_piece(RE_State *st) {
    RE_Piece *res = malloc(sizeof(RE_Piece));
    res->a = parse_atom(st);
    res->min = 1;
    res->max = 1;

    RE_Token *peek = get_token(st);
    if (peek->t == META) {
        if (peek->u.metachar == '*') {
            res->min = 0;
            res->max = -1;
        } else if (peek->u.metachar == '+') {
            res->min = 1;
            res->max = -1;
        } else if (peek->u.metachar == '?') {
            res->min = 0;
            res->max = 1;
        } else {
            unget_token(st, peek);
        }
    } else if (peek->t == BOUND){
        res->min = peek->u.bound[0];
        res->max = peek->u.bound[1];
    } else {
        unget_token(st, peek);
    }

    return res;
}

static RE_Branch *parse_branch(RE_State *st) {
    RE_Branch *res = Branch_new();

    for (;;) {
        RE_Token *peek = get_token(st);

        // jyi's dialect: the outer ')' can be omited
        if (peek->t == END \
                || (peek->t == META && peek->u.metachar == ')') \
                || (peek->t == META && peek->u.metachar == '|')) {
            unget_token(st, peek);
            break;
        }

        unget_token(st, peek);
        if (res->num == res->size) {
            Branch_resize(res);
        }
        res->p[res->num++] = parse_piece(st);
    }

    return res;
}

RE_Node *parse_regex(RE_State *st) {
    RE_Node *res = Node_new();
    int first = true;

    for (;;) {
        RE_Token *peek = get_token(st);

        if (peek->t == END
                || (peek->t == META && peek->u.metachar == ')')) {
            break;
        }

        if (first) {
            unget_token(st, peek);
            if (res->num == res->size) {
                Node_resize(res);
            }
            res->b[res->num++] = parse_branch(st);
            first = false;
        } else {
            if (peek->t == META && peek->u.metachar == '|') {
                if (res->num == res->size) {
                    Node_resize(res);
                }
                res->b[res->num++] = parse_branch(st);
            } else {
                panic("illegal regex");
            }
        }
    }

    return res;
}

#ifdef DRAW_RELATIONSHIP

#define last8bit(ptr) ((unsigned int)(unsigned long)ptr & 0xff)

#define CHARSET_SHOW_MAX 16
void draw_charset(FILE *f, RE_Atom *a) {
    assert(a->is_simple_atom);
    fprintf(f, "    cs%p [label=\"", a);
    bool is_end = true;
    int cnt = 0;
    for (int i = 0; i < 256; ++i) {
        if (a->u.ch[i]) {
            if (isgraph(i)) {
                if (strchr("\"", i)) {
                    fprintf(f, "%c", '\\');
                }
                fprintf(f, "%c", i);
                cnt++;
                if (cnt > CHARSET_SHOW_MAX) {
                    break;
                }
            } else {
                is_end = false;
            }
        }
    }
    if (!is_end) {
        fprintf(f, "...");
    }
    fprintf(f, "\" style=filled];\n");
    fprintf(f, "    cs%p->a%p;\n", a, a);
}

void draw_atom(FILE *f, RE_Atom *a) {
    fprintf(f, "    a%p [label=\"at_%x\" style=dotted];\n", a, last8bit(a));
    if (a->is_simple_atom) {
        draw_charset(f, a);
    } else {
        extern void draw_re(FILE *f, RE_Node *re);
        fprintf(f, "    r%p->a%p;\n", a->u.re, a);
        draw_re(f, a->u.re);
    }
}

void draw_piece(FILE *f, RE_Piece *p) {
    fprintf(f, "    p%p [label=\"pi_%x\" style=dashed];\n", p, last8bit(p));
    if (p->max == -1) {
        fprintf(f, "    a%p->p%p [label=\"%d-%s\"];\n", p->a, p, p->min, "inf");
    } else {
        fprintf(f, "    a%p->p%p [label=\"%d-%d\"];\n", p->a, p, p->min, p->max);
    }
    draw_atom(f, p->a);
}

void draw_branch(FILE *f, RE_Branch *b) {
    fprintf(f, "    b%p [label=\"br_%x\" style=solid];\n", b, last8bit(b));
    for (int i = 0; i < b->num; ++i) {
        fprintf(f, "    p%p->b%p;\n", b->p[i], b);
        draw_piece(f, b->p[i]);
    }
}

void draw_re(FILE *f, RE_Node *re) {
    fprintf(f, "    r%p [label=\"re_%x\" style=bold];\n", re, last8bit(re));
    for (int i = 0; i < re->num; ++i) {
        fprintf(f, "    b%p->r%p;\n", re->b[i], re);
        draw_branch(f, re->b[i]);
    }
}

static void draw_relationship(FILE *f, RE_Node *re, char *regexp) {
    fprintf(f, "digraph Regex_Relationship {\n");
    fprintf(f, "    label=\"regexp = %s\"", regexp);
    fprintf(f, "    labelloc=top;\n");
    fprintf(f, "    labeljust=left;\n");
    draw_re(f, re);
    fprintf(f, "}\n");
}

#endif

typedef struct NFANode NFANode;

typedef struct {
    bool is_epsilon;
    bool ch[256];
    NFANode *next;
} NFAEdge;

#define NFANODE_EDGE_INITIAL_SIZE 4

typedef struct NFANode {
    bool is_begin;
    bool is_end;
    int nedges;
    int edge_size;
    NFAEdge **nexts;
#ifdef DRAW_NFA
    bool visited; // DFS
#endif
} NFANode;

typedef struct {
    NFANode *begin;
    NFANode *end;
} NFAGraph;

NFAEdge *NFAEdge_new(void) {
    NFAEdge *e = (NFAEdge *)alloc(sizeof(NFAEdge));
    fill_by_range(0, 255, e->ch, false);
    e->is_epsilon = false;
    e->next = NULL;
    return e;
}

NFANode *NFANode_nnew(int edge_size) {
    edge_size = edge_size > NFANODE_EDGE_INITIAL_SIZE ? \
                edge_size : NFANODE_EDGE_INITIAL_SIZE;
    NFANode *n = (NFANode *)alloc(sizeof(NFANode));
    n->is_begin = false;
    n->is_end = false;
    n->edge_size = edge_size;
    n->nedges = 0;
    n->nexts = (NFAEdge **)RE_calloc(n->edge_size, sizeof(NFAEdge *));

#ifdef DRAW_NFA
    n->visited = false;
#endif

    return n;
}

#define NFANode_new() \
    NFANode_nnew(NFANODE_EDGE_INITIAL_SIZE)

void NFANode_resize(NFANode *n) {
    n->edge_size = n->edge_size * 2 + 4;
    n->nexts = (NFAEdge **)RE_realloc(n->nexts, n->edge_size, sizeof(NFAEdge *));
}

NFAGraph NFAGraph_new(void) {
    NFAGraph st = {
        .begin = NULL,
        .end = NULL
    };
    return st;
}

NFAGraph NFA_cat(NFAGraph *a, NFAGraph *b) {
    NFANode *end = b->end;
    *a->end = *b->begin;

    // if use epsilon to connect, the graph will be too big
//    NFANode *end = a->end;
//    end->is_end = false;
//    NFAEdge *e = (end->nexts[end->nedges++] = NFAEdge_new());
//    e->next = b->begin;
//    e->is_epsilon = true;
    NFAGraph g = {
        .begin = a->begin,
        .end = end
    };
    return g;
}

NFAGraph NFA_or(NFAGraph *subgraphs, int ngraphs) {
    if (ngraphs == 1) {
        return subgraphs[0];
    }
    NFANode *begin = NFANode_nnew(ngraphs);
    begin->is_begin = true;
    NFANode *end = NFANode_new();
    end->is_end = true;
    for (int i = 0; i < ngraphs; ++i) {
        NFAEdge *e = (begin->nexts[begin->nedges++] = NFAEdge_new());
        e->is_epsilon = true;
        e->next = subgraphs[i].begin;

        NFANode *n = subgraphs[i].end;
        n->is_end = false;
        e = (n->nexts[n->nedges++] = NFAEdge_new()); 
        e->is_epsilon = true;
        e->next = end;
    }

    NFAGraph g = {
        .begin = begin,
        .end = end
    };
    return g;
}

NFAGraph NFA_atom2NFA(RE_Atom *a) {
    NFAGraph g = NFAGraph_new();
    extern NFAGraph NFA_re2NFA(RE_Node *re);
    if (a->is_simple_atom) {
        NFANode *begin = NFANode_new();
        begin->is_begin = true;
        NFAEdge *e = (begin->nexts[begin->nedges++] = NFAEdge_new());
        NFANode *end = NFANode_new();
        end->is_end = true;
        e->next = end;
        memcpy(e->ch, a->u.ch, sizeof(e->ch));
        g.begin = begin;
        g.end = end;
    } else {
        g = NFA_re2NFA(a->u.re);
    }
    return g;
}

NFAGraph NFA_piece2NFA(RE_Piece *p) {
    NFAGraph g = NFAGraph_new();
    if (p->max == 1 && p->min == 1) {
        g = NFA_atom2NFA(p->a);
    } else {
        unimplemented("matchint times only support {1}");
    }
    return g;
}

NFAGraph NFA_branch2NFA(RE_Branch *b) {
    NFAGraph g = NFA_piece2NFA(b->p[0]);
    for (int i = 1; i < b->num; ++i) {
        NFAGraph ng = NFA_piece2NFA(b->p[i]);
        g = NFA_cat(&g, &ng);
    }
    return g;
}

NFAGraph NFA_re2NFA(RE_Node *n) {
    NFAGraph *subgraphs = (NFAGraph *)RE_calloc(n->num, sizeof(NFAGraph));
    for (int i = 0; i < n->num; ++i) {
        subgraphs[i] = NFA_branch2NFA(n->b[i]);
    }
    NFAGraph g = NFA_or(subgraphs, n->num);
    return g;
}

NFAGraph regex2NFA(RE_Node *re) {
    NFAGraph g = NFAGraph_new();
    g = NFA_re2NFA(re);
    return g;
}

#ifdef DRAW_NFA

#define MAXEDGENUM 128
int nedges;
unsigned int edges[MAXEDGENUM][3];

static void push_edge(unsigned int from, unsigned int to, unsigned int label) {
    edges[nedges][0] = from;
    edges[nedges][1] = to;
    edges[nedges][2] = label;
    nedges++;
}

#define last12bit(ptr) ((unsigned int)(unsigned long)(ptr) & 0xfff)

static void draw_NFANode(FILE *f, NFANode *n) {
    if (n == NULL || n->is_end || n->visited) {
        return;
    }

    n->visited = true;
    for (int j = 0; j < n->nedges; ++j) {
        NFAEdge *e = n->nexts[j];
        fprintf(f, "    s_%x->s_%x [label=\"", last12bit(n), \
                last12bit(e->next));
        if (e->is_epsilon) {
            fprintf(f, "ε\"];\n");
        } else {
            bool is_end = true;
            int cnt = 0;
            for (int i = 0; i < 256; ++i) {
                if (e->ch[i]) {
                    if (isgraph(i)) {
                        if (strchr("\"", i)) {
                            fprintf(f, "%c", '\\');
                        }
                        fprintf(f, "%c", i);
                        cnt++;
                        if (cnt > CHARSET_SHOW_MAX) {
                            break;
                        }
                    } else {
                        is_end = false;
                    }
                }
            }
            if (!is_end) {
                fprintf(f, "...");
            }
            fprintf(f, "\"];\n");
        }

        draw_NFANode(f, e->next);
    }
}

static void draw_NFA(FILE *f, NFAGraph *g, char *regexp) {
    fprintf(f, "digraph NFA_Graph {\n");
    fprintf(f, "    label=\"regexp = %s\"", regexp);
    fprintf(f, "    labelloc=top;\n");
    fprintf(f, "    labeljust=left;\n");
    fprintf(f, "    s_%x [shape=doublecircle]\n", last12bit(g->end));
    fprintf(f, "    s_start [style=invis]\n");
    fprintf(f, "    s_start->s_%x [label=\"start\"]\n", last12bit(g->begin));
    draw_NFANode(f, g->begin);
    fprintf(f, "}\n");
}
#endif

int main(int argc, char *argv[]) {
    assert(argc == 2);
    char *regex = argv[1];
    RE_State *st = RES_init(argv[1]);
    RE_Node *re = parse_regex(st);

#ifdef DRAW_RELATIONSHIP
    puts("generate .dot file for relationship...");
    FILE *f = fopen("./relationship.dot", "w");
    draw_relationship(f, re, argv[1]);
    fclose(f);
    puts("OK");
    puts(".dot file is saved at ./relationship.dot");
    puts("");
#endif

    NFAGraph g = NFA_re2NFA(re);
#ifdef DRAW_NFA
    puts("generate .dot file for NFA...");
    f = fopen("./NFA.dot", "w");
    draw_NFA(f, &g, argv[1]);
    fclose(f);
    puts("OK");
    puts(".dot file is saved at ./NFA.dot");
    puts("");
#endif

    free(st);
    return 0;
}
