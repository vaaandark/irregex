#include "NFA.h"

int main(int argc, char *argv[]) {
    assert(argc == 2);
    RE_State *re_st = RES_new(argv[1]);
    RE_Node *re = parse_regex(re_st);
    RES_drop(re_st);

    NFAGraph g = regex2NFA(re);
    Re_drop(re);
#ifdef DRAW_NFA
    puts("\033[1;032mregex:\033[0m generate .dot file for NFA GRAPH...");
    FILE *f = fopen("./NFA-graph.dot", "w");
    draw_NFA(f, &g, argv[1]);
    fclose(f);
    puts("OK");
    puts(".dot file is saved at ./NFA-graph.dot");
    puts("");
#endif

    NFAGraph_clear(&g);
    NFAGraph_drop(&g);

    return 0;
}
