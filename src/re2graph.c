#include "NFA.h"

#define CHARSET_SHOW_MAX 16

void print_charset(FILE *f, bool *ch) {
    bool has_more = true;
    int cnt = 0;
    for (int i = 0; i < 256; ++i) {
        if (ch[i]) {
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
    fprintf(f, "\"];\n");
}

void draw_NFA(FILE *f, NFAGraph *g, char *regexp) {
    fprintf(f, "digraph NFA_Graph {\n");
    fprintf(f, "    label=\"regexp = %s\"\n", regexp);
    fprintf(f, "    labelloc=top;\n");
    fprintf(f, "    labeljust=left;\n");
    fprintf(f, "    s%d [shape=doublecircle]\n", g->end->id);
    fprintf(f, "    s_start [style=invis]\n");
    fprintf(f, "    s_start->s%d [label=\"start\"]\n", g->begin->id);
//    draw_NFANode(f, g->begin);
    for (int k = 0; k < g->num; ++k) {
        NFANode *n = g->nodes[k];
        for (int j = 0; j < n->num; ++j) {
            NFAEdge *e = &n->edges[j];
            fprintf(f, "    s%d->s%d [label=\"", n->id, e->next->id);
            if (e->is_epsilon) {
                fprintf(f, "ε\"];\n");
            } else {
                print_charset(f, e->ch);
            }
        }
    }
    fprintf(f, "}\n");
}

int main(int argc, char *argv[]) {
    assert(argc == 2);
    RE_State *re_st = RES_new(argv[1]);
    RE_Node *re = parse_regex(re_st);
    RES_drop(re_st);

    NFAGraph g = regex2NFA(re);
    Re_drop(re);

    puts("\033[1;032mregex:\033[0m generate .dot file for NFA GRAPH...");
    FILE *f = fopen("./NFA-graph.dot", "w");
    draw_NFA(f, &g, argv[1]);
    fclose(f);
    puts("OK");
    puts(".dot file is saved at ./NFA-graph.dot");
    puts("");

    NFAGraph_clear(&g);
    NFAGraph_drop(&g);

    return 0;
}
