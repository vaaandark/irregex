#include "NFA.h"

int main(int argc, char *argv[]) {
    assert(argc == 2);
    RE_State *re_st = RES_new(argv[1]);
    RE_Node *re = parse_regex(re_st);
    RES_drop(re_st);

#ifdef DRAW_RELATIONSHIP
    puts("\033[1;032mregex:\033[0m generate .dot file for relationship...");
    FILE *f = fopen("./relationship.dot", "w");
    draw_relationship(f, re, argv[1]);
    fclose(f);
    puts("OK");
    puts(".dot file is saved at ./relationship.dot");
    puts("");
#endif

    NFAGraph g = regex2NFA(re);
    Re_drop(re);
    NFAGraph h = NFAGraph_clone(&g);
#ifdef DRAW_NFA
    puts("\033[1;032mregex:\033[0m generate .dot file for NFA...");
    f = fopen("./NFA.dot", "w");
    draw_NFA(f, &g, argv[1]);
    fclose(f);
    puts("OK");
    puts(".dot file is saved at ./NFA.dot");
    puts("");

    puts("\033[1;032mregex:\033[0m generate .dot file for NFA's copy...");
    f = fopen("./NFA_copy.dot", "w");
    draw_NFA(f, &h, argv[1]);
    fclose(f);
    puts("OK");
    puts(".dot file is saved at ./NFA_copy.dot");
    puts("");
#endif

    NFAGraph_clear(&g);
    NFAGraph_drop(&g);
    NFAGraph_clear(&h);
    NFAGraph_drop(&h);

    return 0;
}
