#include "NFA.h"

#ifdef DRAW_RELATIONSHIP

#define last8bit(ptr) ((unsigned int)(unsigned long)ptr & 0xff)

void draw_charset(FILE *f, RE_Atom *a) {
    assert(a->is_simple_atom);
    fprintf(f, "    cs%p [label=\"", a);
    bool has_more = true;
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
                has_more = false;
            }
        }
    }
    if (!has_more) {
        fprintf(f, "...");
    }
    fprintf(f, "\" style=filled];\n");
    fprintf(f, "    cs%p->a%p;\n", a, a);
}

void draw_re(FILE *, RE_Node *);

void draw_atom(FILE *f, RE_Atom *a) {
    fprintf(f, "    a%p [label=\"at_%x\" style=dotted];\n", a, last8bit(a));
    if (a->is_simple_atom) {
        draw_charset(f, a);
    } else {
        fprintf(f, "    r%p->a%p;\n", a->u.re, a);
        draw_re(f, a->u.re);
    }
}

void draw_piece(FILE *f, RE_Piece *p) {
    fprintf(f, "    p%p [label=\"pi_%x\" style=dashed];\n", p, last8bit(p));
    if (p->max == p->min) {
        fprintf(f, "    a%p->p%p [label=\"%d\"];\n", p->a, p, p->min);
    } else if (p->max == -1 ){
        fprintf(f, "    a%p->p%p [label=\"%d - %s\"];\n", p->a, p, p->min, "inf");
    } else {
        fprintf(f, "    a%p->p%p [label=\"%d - %d\"];\n", p->a, p, p->min, p->max);
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

void draw_relationship(FILE *f, RE_Node *re, char *regexp) {
    fprintf(f, "digraph Regex_Relationship {\n");
    fprintf(f, "    label=\"regexp = %s\"", regexp);
    fprintf(f, "    labelloc=top;\n");
    fprintf(f, "    labeljust=left;\n");
    draw_re(f, re);
    fprintf(f, "}\n");
}

#endif

int main(int argc, char *argv[]) {
    assert(argc == 2);
    RE_State *re_st = RES_new(argv[1]);
    RE_Node *re = parse_regex(re_st);
    RES_drop(re_st);

#ifdef DRAW_RELATIONSHIP
    puts("\033[1;032mregex:\033[0m generate .dot file for RE tree...");
    FILE *f = fopen("./RE-tree.dot", "w");
    draw_relationship(f, re, argv[1]);
    fclose(f);
    puts("OK");
    puts(".dot file is saved at ./RE-tree.dot");
    puts("");
#endif

    Re_drop(re);
    return 0;
}
