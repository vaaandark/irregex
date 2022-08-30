#ifndef NFA_H_
#define NFA_H_

#include "re.h"

typedef struct NFANode NFANode;
typedef struct {
    bool is_epsilon;
    bool ch[256];
    NFANode *next;
} NFAEdge;

#define NFANODE_EDGE_INITIAL_SIZE 4

typedef struct NFANode {
    int id;
    bool is_end;
    int num;
    int size;
    NFAEdge *edges;
#ifdef DRAW_NFA
    bool visited_on_draw; // DFS
#endif
} NFANode;

typedef struct {
    NFANode *begin;
    NFANode *end;
    NFANode **nodes; // internal use only
    int num; // internal use only
    int size; // internal use only
} NFAGraph;

extern void NFAGraph_clear(NFAGraph *g);
extern void NFAGraph_drop(NFAGraph *g);

extern NFAGraph NFA_re2NFA(RE_Node *n);
extern NFAGraph NFAGraph_clone(NFAGraph *g);
#define regex2NFA(re) NFA_re2NFA(re)

#ifdef DRAW_NFA
#define CHARSET_SHOW_MAX 16
extern void draw_NFA(FILE *f, NFAGraph *g, char *regexp);
#endif

#endif
