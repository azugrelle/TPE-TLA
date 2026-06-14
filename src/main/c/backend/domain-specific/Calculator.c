#include "Calculator.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void _shutdownCalculatorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Calculator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeCalculatorModule(void) {
	_logger = createLogger("Calculator");
	return _shutdownCalculatorModule;
}

/* PRIVATE FUNCTIONS */

#define MINUTES_PER_HOUR 60
#define MINUTES_PER_DAY  1440
#define HOURS_PER_DAY    24
#define INITIAL_CAPACITY 4

static void _evaluateInstructionList(InstructionList * list, ClockStateList * states, int * activeIndex);

/** The style every clock starts with before any styling instruction is applied. */
static StyleState _defaultStyle(void) {
	StyleState style = {
		.handColor = COLOR_BLACK,
		.bgColor = COLOR_WHITE,
		.borderColor = COLOR_BLACK,
		.numbers = NUMBER_WESTERN
	};
	return style;
}

/** Wraps a (possibly negative or overflowing) minute-of-day into [0, 1440). */
static int _normalizeMinutes(int totalMinutes) {
	return ((totalMinutes % MINUTES_PER_DAY) + MINUTES_PER_DAY) % MINUTES_PER_DAY;
}

/**
 * Appends a freshly-declared clock to the list, growing it if necessary, and
 * returns the index of the new entry (or -1 on allocation failure).
 */
static int _appendClock(ClockStateList * states, const char * name, int hour, int minute) {
	if (states->count == states->capacity) {
		int newCapacity = states->capacity == 0 ? INITIAL_CAPACITY : states->capacity * 2;
		ClockState * grown = realloc(states->clocks, newCapacity * sizeof(ClockState));
		if (grown == NULL) {
			logError(_logger, "Unable to grow the clock state list to %d entries.", newCapacity);
			return -1;
		}
		states->clocks = grown;
		states->capacity = newCapacity;
	}
	char * ownedName = strdup(name);
	if (ownedName == NULL) {
		logError(_logger, "Unable to copy the clock name '%s'.", name);
		return -1;
	}
	ClockState * clock = &states->clocks[states->count];
	clock->name = ownedName;
	clock->hour = hour;
	clock->minute = minute;
	clock->style = _defaultStyle();
	clock->rendered = false;
	return states->count++;
}

/** Evaluates a boolean condition against the current state of the active clock. */
static bool _evaluateCondition(Condition * condition, const ClockState * state) {
	switch (condition->type) {
		case COND_SIMPLE: {
			int lhs = (condition->simple.component == COMP_HOUR) ? state->hour : state->minute;
			switch (condition->simple.comparator) {
				case CMP_EQ:  return lhs == condition->simple.value;
				case CMP_NEQ: return lhs != condition->simple.value;
				case CMP_LT:  return lhs <  condition->simple.value;
				case CMP_GT:  return lhs >  condition->simple.value;
				case CMP_LTE: return lhs <= condition->simple.value;
				case CMP_GTE: return lhs >= condition->simple.value;
			}
			return false;
		}
		case COND_AND:
			return _evaluateCondition(condition->binary.left, state)
				&& _evaluateCondition(condition->binary.right, state);
		case COND_OR:
			return _evaluateCondition(condition->binary.left, state)
				|| _evaluateCondition(condition->binary.right, state);
		case COND_NOT:
			return !_evaluateCondition(condition->unary.operand, state);
	}
	return false;
}

/**
 * Applies a single instruction to the simulation. The active clock is tracked
 * by index (not by pointer) so that growing the list never leaves a dangling
 * reference behind.
 */
static void _evaluateInstruction(Instruction * instruction, ClockStateList * states, int * activeIndex) {
	switch (instruction->type) {
		case INSTR_CLOCK: {
			int index = _appendClock(states, instruction->clock.name, instruction->clock.hour, instruction->clock.minute);
			if (index >= 0) {
				*activeIndex = index;
			}
			break;
		}
		case INSTR_ADD:
		case INSTR_SUB: {
			ClockState * state = &states->clocks[*activeIndex];
			int delta = (instruction->arithmetic.unit == HOURS_UNIT)
				? instruction->arithmetic.value * MINUTES_PER_HOUR
				: instruction->arithmetic.value;
			if (instruction->type == INSTR_SUB) {
				delta = -delta;
			}
			int totalMinutes = _normalizeMinutes(state->hour * MINUTES_PER_HOUR + state->minute + delta);
			state->hour = totalMinutes / MINUTES_PER_HOUR;
			state->minute = totalMinutes % MINUTES_PER_HOUR;
			break;
		}
		case INSTR_SET_HOUR: {
			states->clocks[*activeIndex].hour = instruction->setTime.value;
			break;
		}
		case INSTR_SET_MINUTE: {
			states->clocks[*activeIndex].minute = instruction->setTime.value;
			break;
		}
		case INSTR_ROUND: {
			ClockState * state = &states->clocks[*activeIndex];
			int remainder = state->minute % 5;
			if (remainder < 3) {
				state->minute -= remainder;
			} else {
				state->minute += (5 - remainder);
			}
			if (state->minute >= MINUTES_PER_HOUR) {
				state->minute -= MINUTES_PER_HOUR;
				state->hour = (state->hour + 1) % HOURS_PER_DAY;
			}
			break;
		}
		case INSTR_NEXT_HOUR: {
			ClockState * state = &states->clocks[*activeIndex];
			state->minute = 0;
			state->hour = (state->hour + 1) % HOURS_PER_DAY;
			break;
		}
		case INSTR_TIMEZONE: {
			ClockState * state = &states->clocks[*activeIndex];
			int offset = instruction->timezone.toOffset - instruction->timezone.fromOffset;
			state->hour = ((state->hour + offset) % HOURS_PER_DAY + HOURS_PER_DAY) % HOURS_PER_DAY;
			break;
		}
		case INSTR_COLOR: {
			states->clocks[*activeIndex].style.handColor = instruction->style.color;
			break;
		}
		case INSTR_BACKGROUND: {
			states->clocks[*activeIndex].style.bgColor = instruction->style.color;
			break;
		}
		case INSTR_BORDER: {
			states->clocks[*activeIndex].style.borderColor = instruction->style.color;
			break;
		}
		case INSTR_NUMBERS: {
			states->clocks[*activeIndex].style.numbers = instruction->numbers.type;
			break;
		}
		case INSTR_REPEAT: {
			for (int i = 0; i < instruction->repeat.times; ++i) {
				_evaluateInstructionList(instruction->repeat.body, states, activeIndex);
			}
			break;
		}
		case INSTR_IF: {
			if (_evaluateCondition(instruction->ifInstr.condition, &states->clocks[*activeIndex])) {
				_evaluateInstructionList(instruction->ifInstr.thenBranch, states, activeIndex);
			} else if (instruction->ifInstr.elseBranch != NULL) {
				_evaluateInstructionList(instruction->ifInstr.elseBranch, states, activeIndex);
			}
			break;
		}
		case INSTR_RENDER: {
			for (int i = 0; i < states->count; ++i) {
				states->clocks[i].rendered = true;
			}
			break;
		}
	}
}

static void _evaluateInstructionList(InstructionList * list, ClockStateList * states, int * activeIndex) {
	while (list != NULL) {
		_evaluateInstruction(list->instruction, states, activeIndex);
		list = list->next;
	}
}

/* PUBLIC FUNCTIONS */

ClockStateList * calculateClockStates(Program * program) {
	logDebugging(_logger, "Calculating final clock states...");
	ClockStateList * states = calloc(1, sizeof(ClockStateList));
	if (states == NULL) {
		logError(_logger, "Unable to allocate the clock state list.");
		return NULL;
	}
	int activeIndex = -1;
	_evaluateInstructionList(program->instructions, states, &activeIndex);
	logDebugging(_logger, "Calculated the final state of %d clock(s).", states->count);
	return states;
}

void destroyClockStateList(ClockStateList * list) {
	if (list == NULL) {
		return;
	}
	for (int i = 0; i < list->count; ++i) {
		free(list->clocks[i].name);
	}
	free(list->clocks);
	free(list);
}
