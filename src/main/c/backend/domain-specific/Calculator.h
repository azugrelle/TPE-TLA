#ifndef CALCULATOR_HEADER
#define CALCULATOR_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>

/** Initialize module's internal state. */
ModuleDestructor initializeCalculatorModule(void);

/**
 * The resolved visual style of a clock, after applying every styling
 * instruction that affects it. Holds the values used later by the generator.
 */
typedef struct {
	Color handColor;       // default: COLOR_BLACK
	Color bgColor;         // default: COLOR_WHITE
	Color borderColor;     // default: COLOR_BLACK
	NumberType numbers;    // default: NUMBER_ARABIC
} StyleState;

/**
 * The final state of a single named clock once the whole program has been
 * simulated. The "name" is owned by this structure (a private copy).
 */
typedef struct {
	char * name;
	int hour;              // 0–23
	int minute;            // 0–59
	StyleState style;
	bool rendered;         // true if a 'render' was reached after this clock was declared
} ClockState;

/**
 * A dynamic array holding the final state of every clock declared by the
 * program, in declaration order.
 */
typedef struct {
	ClockState * clocks;
	int count;
	int capacity;
} ClockStateList;

/**
 * Simulates the execution of the program and returns the final state of every
 * clock it declares. The input AST is expected to be semantically valid (see
 * the semantic-analysis phase). Returns NULL only on allocation failure; the
 * result must be released with destroyClockStateList.
 */
ClockStateList * calculateClockStates(Program * program);

/** Releases a ClockStateList and every resource it owns. */
void destroyClockStateList(ClockStateList * list);

#endif
