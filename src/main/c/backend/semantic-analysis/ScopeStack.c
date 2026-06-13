#include "ScopeStack.h"

ScopeStack * scopeStackCreate(void) {
	return calloc(1, sizeof(ScopeStack));
}

void scopeStackDestroy(ScopeStack * stack) {
	if (stack == NULL) return;
	ScopeFrame * current = stack->top;
	while (current != NULL) {
		ScopeFrame * next = current->next;
		free(current);
		current = next;
	}
	free(stack);
}

void scopeStackPush(ScopeStack * stack, const char * activeClockName) {
	ScopeFrame * frame = calloc(1, sizeof(ScopeFrame));
	frame->activeClockName = activeClockName;
	frame->next = stack->top;
	stack->top = frame;
}

void scopeStackPop(ScopeStack * stack) {
	if (stack->top == NULL) return;
	ScopeFrame * frame = stack->top;
	stack->top = frame->next;
	free(frame);
}

const char * scopeStackActiveClockName(ScopeStack * stack) {
	if (stack->top == NULL) return NULL;
	return stack->top->activeClockName;
}
