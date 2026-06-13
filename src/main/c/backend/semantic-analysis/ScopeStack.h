#ifndef SCOPE_STACK_HEADER
#define SCOPE_STACK_HEADER

#include <stdlib.h>

typedef struct ScopeFrame ScopeFrame;

struct ScopeFrame {
	const char * activeClockName;
	ScopeFrame * next;
};

typedef struct {
	ScopeFrame * top;
} ScopeStack;

ScopeStack * scopeStackCreate(void);
void         scopeStackDestroy(ScopeStack * stack);
void         scopeStackPush(ScopeStack * stack, const char * activeClockName);
void         scopeStackPop(ScopeStack * stack);
const char * scopeStackActiveClockName(ScopeStack * stack);

#endif
