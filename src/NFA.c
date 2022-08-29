#include "NFA.h"

static NFAEdge *NFAEdge_new(void) {
    NFAEdge *e = (NFAEdge *)alloc(sizeof(NFAEdge));
    fill_by_range(0, 255, e->ch, false);
    e->is_epsilon = false;
    e->next = NULL;
    return e;
}

static NFANode *NFANode_nnew(int size) {
    size = size > NFANODE_EDGE_INITIAL_SIZE ? \
                size : NFANODE_EDGE_INITIAL_SIZE;
    NFANode *n = (NFANode *)alloc(sizeof(NFANode));
    n->is_end = false;
    n->size = size;
    n->num = 0;
    n->u.edges = (NFAEdge **)RE_calloc(n->size, sizeof(NFAEdge *));

#ifdef DRAW_NFA
    n->visited = false;
#endif

    return n;
}

#define NFANode_new() \
    NFANode_nnew(NFANODE_EDGE_INITIAL_SIZE)

static void NFANode_resize(NFANode *n) {
    n->size = n->size * 2 + 4;
    n->u.edges = RE_realloc(n->u.edges, n->size, sizeof(NFAEdge *));
}

NFAGraph NFAGraph_new(void) {
    NFAGraph st = {
        .begin = NULL,
        .end = NULL
    };
    return st;
}

static NFAGraph NFA_cat(NFAGraph *a, NFAGraph *b) {
    NFANode *end = b->end;
    *a->end = *b->begin;

    NFAGraph g = {
        .begin = a->begin,
        .end = end
    };
    return g;
}

static void NFA_merge(NFANode *dst, NFANode *src) {
    int nd = dst->num;
    int ns = src->num;
    if (nd + ns >= dst->size) {
        dst->size = nd + ns + 4;
        dst->u.edges = (NFAEdge **)RE_realloc(dst->u.edges, dst->size, sizeof(NFAEdge *));
    }
    for (int i = nd, j = 0; j < ns; ++i, ++j) {
        dst->u.edges[i] = src->u.edges[j];
    }
    dst->num = nd + ns;
}

static NFAGraph NFA_or(NFAGraph *subgraphs, int ngraphs) {
    if (ngraphs == 1) {
        return subgraphs[0];
    }
    NFANode *begin = NFANode_nnew(ngraphs);
    NFANode *end = NFANode_new();
    end->is_end = true;
    for (int i = 0; i < ngraphs; ++i) {
        NFA_merge(begin, subgraphs[i].begin);

        NFANode *subend = subgraphs[i].end;
        NFA_merge(end, subgraphs[i].end);
    }

    for (int j = 0; j < end->num; ++j) {
        NFANode *back = end->u.backs[j];
        bool *ch = back->u.edges[0]->ch;
        for (int i = 0; i < 256; ++i) {
            if (ch[i])
                putchar(i);
        }
        for (int k = 0; k < back->num; ++k) {
            if (back->u.edges[k]->next->is_end) {
                back->u.edges[k]->next = end;
            }
        }
    }

    NFAGraph g = {
        .begin = begin,
        .end = end
    };
    return g;
}

static NFAGraph NFA_re2NFA(RE_Node *);
static NFAGraph NFA_atom2NFA(RE_Atom *a) {
    NFAGraph g = NFAGraph_new();
    if (a->is_simple_atom) {
        NFANode *begin = NFANode_new();
        NFAEdge *e = (begin->u.edges[begin->num++] = NFAEdge_new());
        NFANode *end = NFANode_new();
        end->is_end = true;
        e->next = end;
        end->u.backs[end->num++] = begin;
        memcpy(e->ch, a->u.ch, sizeof(e->ch));
        g.begin = begin;
        g.end = end;
    } else {
        g = NFA_re2NFA(a->u.re);
    }
    return g;
}

static NFAGraph NFA_piece2NFA(RE_Piece *p) {
    NFAGraph g = NFAGraph_new();
    if (p->max == 1 && p->min == 1) {
        g = NFA_atom2NFA(p->a);
    } else {
        unimplemented("matchint times only support {1}");
    }
    return g;
}

static NFAGraph NFA_branch2NFA(RE_Branch *b) {
    NFAGraph g = NFA_piece2NFA(b->p[0]);
    for (int i = 1; i < b->num; ++i) {
        NFAGraph ng = NFA_piece2NFA(b->p[i]);
        g = NFA_cat(&g, &ng);
    }
    return g;
}

static NFAGraph NFA_re2NFA(RE_Node *n) {
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

#define last12bit(ptr) ((unsigned int)(unsigned long)(ptr) & 0xfff)

static void draw_NFANode(FILE *f, NFANode *n) {
    if (n == NULL || n->is_end || n->visited) {
        return;
    }

    n->visited = true;
    for (int j = 0; j < n->num; ++j) {
        NFAEdge *e = n->u.edges[j];
        fprintf(f, "    s_%x->s_%x [label=\"", last12bit(n), \
                last12bit(e->next));
        if (e->is_epsilon) {
            fprintf(f, "ε\"];\n");
        } else {
            bool has_more = true;
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
                        has_more = false;
                    }
                }
            }
            if (!has_more) {
                fprintf(f, "...");
            }
            fprintf(f, "\"];\n");
        }

        draw_NFANode(f, e->next);
    }
}

void draw_NFA(FILE *f, NFAGraph *g, char *regexp) {
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

