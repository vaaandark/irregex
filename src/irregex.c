#include "NFA.h"

NFAGraph regex_compile(const char *regex) {
    RE_State *st = RES_new(regex);
    RE_Node *re = parse_regex(st);
    RES_drop(st);
    NFAGraph g = regex2NFA(re);
    Re_drop(re);
    return g;
}

static int regex_execute_rec(NFANode *n, const char *pos, bool *matched,
        const char **last_visited_pos) {
    if (n->is_end) {
        *matched = true;
    }

    // non-trans circle exists
    if (last_visited_pos[n->id] == pos) {
        return 0;
    }

    int len = 0, res = 0;
    for (int i = 0; i < n->num; ++i) {
        NFAEdge *e = &n->edges[i];
        last_visited_pos[n->id] = pos;
        if (e->is_epsilon) {
            res = regex_execute_rec(e->next, pos, matched, last_visited_pos);
        } else {
            if (e->ch[((int)*pos + 256) % 256]) {
                res = 1 + regex_execute_rec(e->next, pos + 1, matched,
                        last_visited_pos);
            }
        }
        len = len > res ? len : res;
    }

    return len;
}

int regex_execute(NFAGraph *g, const char *str) {
    bool matched = false;
    const char **last_visited_pos = (const char **)RE_calloc(g->num,
            sizeof(const char *));
    int len = regex_execute_rec(g->begin, str, &matched, last_visited_pos);
    free(last_visited_pos);
    return matched ? len : -1;
}

void regex_free(NFAGraph *g) {
    NFAGraph_clear(g);
    NFAGraph_drop(g);
}

