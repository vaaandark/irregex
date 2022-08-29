#include "NFA.h"

int main(int argc, char *argv[]) {
    assert(argc == 2);
    RE_State *st = RES_init(argv[1]);
    RE_Node *re = parse_regex(st);
    RES_drop(st);

#ifdef DRAW_RELATIONSHIP
    puts("generate .dot file for relationship...");
    FILE *f = fopen("./relationship.dot", "w");
    draw_relationship(f, re, argv[1]);
    fclose(f);
    puts("OK");
    puts(".dot file is saved at ./relationship.dot");
    puts("");
#endif

#ifdef DRAW_NFA
    NFAGraph g = regex2NFA(re);
    puts("generate .dot file for NFA...");
    f = fopen("./NFA.dot", "w");
    draw_NFA(f, &g, argv[1]);
    fclose(f);
    puts("OK");
    puts(".dot file is saved at ./NFA.dot");
    puts("");
#endif

    Re_drop(re);

    return 0;
}
