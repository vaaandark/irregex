#include "NFA.h"

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
