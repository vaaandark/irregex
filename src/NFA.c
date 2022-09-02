#include "NFA.h"

#define NFANODE_INITIAL_SIZE 64

static NFANode *NFANode_nnew(int size) {
    size = size > NFANODE_EDGE_INITIAL_SIZE ?
                size : NFANODE_EDGE_INITIAL_SIZE;
    NFANode *n = (NFANode *)alloc(sizeof(NFANode));
    n->is_end = false;
    n->size = size;
    n->num = 0;
    n->edges = (NFAEdge *)RE_calloc(n->size, sizeof(NFAEdge));
    return n;
}

NFANode *NFANode_new(void) {
    return NFANode_nnew(NFANODE_EDGE_INITIAL_SIZE);
}

static void NFANode_resize(NFANode *n) {
    n->size = n->size * 2 + 4;
    n->edges = (NFAEdge *)RE_realloc(n->edges, n->size, sizeof(NFAEdge));
}

void NFANode_drop(NFANode *n) {
    if (n->edges != NULL && n->size != 0) {
        free(n->edges);
    }
    free(n);
}

#define NFAGRAPH_INITIAL_SIZE 256
NFAGraph NFAGraph_nnew(int size) {
    return (NFAGraph) {
        .begin = NULL,
        .end = NULL,
        .size = size,
        .nodes = (NFANode **)RE_calloc(size, sizeof(NFANode *)),
        .num = 0
    };
}

NFAGraph NFAGraph_new(void) {
    return NFAGraph_nnew(NFAGRAPH_INITIAL_SIZE);
}

void NFAGraph_clear(NFAGraph *g) {
    for (int i = 0; i < g->num; ++i) {
        NFANode_drop(g->nodes[i]);
    }
    g->num = 0;
}

// after calling this, NFAGraph's nodes, num, size are invalid
void NFAGraph_drop(NFAGraph *g) {
    g->num = g->size = 0;
    free(g->nodes);
    g->nodes = NULL;
}

static void NFAGraph_resize(NFAGraph *g) {
    g->size = g->size * 2 + 4;
    g->nodes = (NFANode **)RE_realloc(g->nodes, g->size, sizeof(NFANode *));
}

static void NFAGraph_add_node(NFAGraph *g, NFANode *n) {
    if (g->num == g->size) {
        NFAGraph_resize(g);
    }
    g->nodes[g->num] = n;
    n->id = g->num;
    g->num++;
}

static void NFAGraph_add_node_when_cloning(NFAGraph *g, NFANode *n) {
    g->nodes[n->id] = n;
}

static void NFANode_add_epsilon_edge(NFANode *from, NFANode *to) {
    if (from->num == from->size) {
        NFANode_resize(from);
    }
    from->edges[from->num].is_epsilon = true;
    from->edges[from->num++].next = to;
}

static void NFANode_add_charset_edge(NFANode *from, NFANode *to, bool *ch) {
    if (from->num == from->size) {
        NFANode_resize(from);
    }
    from->edges[from->num].is_epsilon = false;
    memcpy(from->edges[from->num].ch, ch, 256);
    from->edges[from->num++].next = to;
}

static NFANode *NFANode_clone(NFANode *n) {
    NFANode *nn = NFANode_nnew(n->size);
    nn->id = n->id;
    nn->is_end = n->is_end;
    return nn;
}

static void NFAGraph_deep_clone(NFAGraph *g, NFANode *n, bool *visited) {
    if (visited[n->id]) {
        return;
    }
    visited[n->id] = true;

    NFANode *nn = NFANode_clone(n);
    g->nodes[n->id] = nn;

    NFAGraph_add_node_when_cloning(g, nn);

    if (n->is_end) {
        g->end = nn;
    }
    for (int i = 0; i < n->num; ++i) {
        NFAEdge *e = &n->edges[i];
        NFANode *next;
        if (!visited[e->next->id]) { // need to alloc
            NFAGraph_deep_clone(g, e->next, visited);
        }
        next = g->nodes[e->next->id];
        if (e->is_epsilon) {
            NFANode_add_epsilon_edge(nn, next);
        } else {
            NFANode_add_charset_edge(nn, next, e->ch);
        }
    }
}

NFAGraph NFAGraph_clone(NFAGraph *g) {
    bool *visited = RE_calloc(g->num, sizeof(bool));
    NFAGraph ng = {
        .nodes = (NFANode **)RE_calloc(g->size, sizeof(NFANode *)),
        .size = g->size,
        .num = g->num,
    };
    NFAGraph_deep_clone(&ng, g->begin, visited);
    ng.begin = ng.nodes[0];
    free(visited);
    return ng;
}

// drop src when calling this func
static void NFAGraph_merge(NFAGraph *dst, NFAGraph *src) {
    int nd = dst->num;
    int ns = src->num;
    if (nd + ns > dst->size) {
        dst->size = (dst->num + src->num) * 2 + 4;
        dst->nodes = (NFANode **)RE_realloc(dst->nodes, dst->size,
                sizeof(NFANode *));
    }
    for (int i = nd, j = 0; j < ns; ++i, ++j) {
        dst->nodes[i] = src->nodes[j];
        dst->nodes[i]->id = i;
    }
    dst->num = nd + ns;
    NFAGraph_drop(src);
}

static inline void unset_end(NFANode *n) {
    n->is_end = false;
}

static inline void set_end(NFANode *n) {
    n->is_end = true;
}

static NFAGraph NFA_cat(NFAGraph *front, NFAGraph *back) {
    unset_end(front->end);
    NFANode_add_epsilon_edge(front->end, back->begin);
    front->end = back->end;
    NFAGraph_merge(front, back);
    return *front;
}

static NFAGraph NFA_or(NFAGraph *subgraphs, int ngraphs) {
    if (ngraphs == 1) {
        return subgraphs[0];
    }

    NFAGraph g = NFAGraph_new();

    NFANode *begin = NFANode_nnew(ngraphs);
    NFANode *end = NFANode_nnew(ngraphs);
    NFAGraph_add_node(&g, begin);
    NFAGraph_add_node(&g, end);
    set_end(end);
    for (int i = 0; i < ngraphs; ++i) {
        NFANode_add_epsilon_edge(begin, subgraphs[i].begin);
        unset_end(subgraphs[i].end);
        NFANode_add_epsilon_edge(subgraphs[i].end, end);
        NFAGraph_merge(&g, &subgraphs[i]);
    }

    g.begin = begin;
    g.end = end;
    return g;
}

extern NFAGraph NFA_re2NFA(RE_Node *);
static NFAGraph NFA_atom2NFA(RE_Atom *a) {
    NFAGraph g;
    if (a->is_simple_atom) {
        g = NFAGraph_new();
        NFANode *begin = NFANode_new();
        NFANode *end = NFANode_new();
        set_end(end);
        NFAGraph_add_node(&g, begin);
        NFAGraph_add_node(&g, end);
        NFANode_add_charset_edge(begin, end, a->u.ch);
        g.begin = begin;
        g.end = end;
    } else {
        g = NFA_re2NFA(a->u.re);
    }
    return g;
}

static NFAGraph NFAGraph_nclone(NFAGraph *g, int times) {
    NFAGraph gcpy = NFAGraph_clone(g);
    for (int i = 0; i < times; ++i) {
        NFAGraph tmp = NFAGraph_clone(&gcpy);
        NFA_cat(g, &tmp);
    }
    NFAGraph_clear(&gcpy);
    NFAGraph_drop(&gcpy);
    return *g;
}

static NFAGraph NFA_piece2NFA(RE_Piece *p) {
    NFAGraph g = NFA_atom2NFA(p->a);
    if (p->max == 1 && p->min == 1) {
        // do nothing
    } else if (p->max == -1 && p->min == 0) { // {0, inf}
        NFANode_add_epsilon_edge(g.begin, g.end);
        NFANode_add_epsilon_edge(g.end, g.begin);
    } else if (p->max == -1) { // {n, inf}
        // for {n, inf} like {3, inf}
        // consider it as '(aaa)a*'
        NFAGraph tail = NFAGraph_clone(&g);
        NFANode_add_epsilon_edge(tail.begin, tail.end);
        NFANode_add_epsilon_edge(tail.end, tail.begin);
        if (p->min > 1) {
            g = NFAGraph_nclone(&g, p->min - 1);
        }
        NFA_cat(&g, &tail);
    } else if (p->max == p->min) { // {n}
        g = NFAGraph_nclone(&g, p->min - 1);
    } else if (p->max == 1 && p->min == 0) { // {0, 1} ?
        NFANode_add_epsilon_edge(g.begin, g.end);
    } else { // {n, m}
        NFANode **begins = (NFANode **)RE_calloc(p->max - p->min,
                sizeof(NFANode *));
        NFAGraph gcpy = NFAGraph_clone(&g);

        for (int i = 1; i <= p->min - 1; ++i) {
            NFAGraph tmp = NFAGraph_clone(&gcpy);
            NFA_cat(&g, &tmp);
        }
        for (int i = p->min, j = 0; i <= p->max - 1; ++i, ++j) {
            NFAGraph tmp = NFAGraph_clone(&gcpy);
            begins[j] = tmp.begin;
            NFA_cat(&g, &tmp);
        }

        for (int i = 0; i < p->max - p->min; ++i) {
            NFANode_add_epsilon_edge(begins[i], g.end);
        }

        NFAGraph_clear(&gcpy);
        NFAGraph_drop(&gcpy);
        free(begins);
    }
    return g;
}

static NFAGraph NFA_branch2NFA(RE_Branch *b) {
    NFAGraph g = NFA_piece2NFA(b->p[0]);
    for (int i = 1; i < b->num; ++i) {
        NFAGraph ng = NFA_piece2NFA(b->p[i]);
        g = NFA_cat(&g, &ng);
        NFAGraph_merge(&g, &ng);
    }
    return g;
}

NFAGraph NFA_re2NFA(RE_Node *n) {
    NFAGraph *subgraphs = (NFAGraph *)RE_calloc(n->num, sizeof(NFAGraph));
    for (int i = 0; i < n->num; ++i) {
        subgraphs[i] = NFA_branch2NFA(n->b[i]);
    }
    NFAGraph g = NFA_or(subgraphs, n->num);
    free(subgraphs);
    return g;
}

