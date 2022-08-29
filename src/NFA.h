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
    bool is_end;
    int num;
    int size;
    union {
        NFANode **backs;
        NFAEdge **edges;
    } u; // when the node is the end one, store its pre
#ifdef DRAW_NFA
    bool visited; // DFS
#endif
} NFANode;

typedef struct {
    NFANode *begin;
    NFANode *end;
} NFAGraph;

extern NFAGraph regex2NFA(RE_Node *re);

#ifdef DRAW_NFA
#define CHARSET_SHOW_MAX 16
extern void draw_NFA(FILE *f, NFAGraph *g, char *regexp);
#endif

#endif
