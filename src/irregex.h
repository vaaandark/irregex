#include "NFA.h"

NFAGraph regex_compile(const char *regex);

// return -1: not match
// return a nonnegative number: length of match characters
int regex_execute(NFAGraph *g, const char *str);

void regex_free(NFAGraph *g);

