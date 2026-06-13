#include "SemanticAnalyzer.h"
#include "ScopeStack.h"
#include "SymbolTable.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void _shutdownSemanticAnalyzerModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: SemanticAnalyzer...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeSemanticAnalyzerModule(void) {
	_logger = createLogger("SemanticAnalyzer");
	return _shutdownSemanticAnalyzerModule;
}

/* PRIVATE FUNCTIONS */

static bool _analyzeInstructionList(InstructionList * list, SymbolTable * table, ScopeStack * scopes, const char ** activeClock);

static bool _analyzeInstruction(Instruction * instruction, SymbolTable * table, ScopeStack * scopes, const char ** activeClock) {
	bool valid = true;
	switch (instruction->type) {
		case INSTR_CLOCK: {
			if (!symbolTableInsert(table, instruction->clock.name)) {
				logError(_logger, "Clock '%s' was already declared.", instruction->clock.name);
				valid = false;
			}
			*activeClock = instruction->clock.name;
			break;
		}
		case INSTR_SET_HOUR: {
			if (*activeClock == NULL) {
				logError(_logger, "Operation 'set hour' used before declaring a clock.");
				valid = false;
			}
			if (instruction->setTime.value < 0 || instruction->setTime.value > 23) {
				logError(_logger, "Hour value out of range [0, 23]: %d.", instruction->setTime.value);
				valid = false;
			}
			break;
		}
		case INSTR_SET_MINUTE: {
			if (*activeClock == NULL) {
				logError(_logger, "Operation 'set minute' used before declaring a clock.");
				valid = false;
			}
			if (instruction->setTime.value < 0 || instruction->setTime.value > 59) {
				logError(_logger, "Minute value out of range [0, 59]: %d.", instruction->setTime.value);
				valid = false;
			}
			break;
		}
		case INSTR_TIMEZONE: {
			if (*activeClock == NULL) {
				logError(_logger, "Operation 'timezone' used before declaring a clock.");
				valid = false;
			}
			if (instruction->timezone.fromOffset < -12 || instruction->timezone.fromOffset > 14) {
				logError(_logger, "UTC offset out of range [-12, 14]: %d.", instruction->timezone.fromOffset);
				valid = false;
			}
			if (instruction->timezone.toOffset < -12 || instruction->timezone.toOffset > 14) {
				logError(_logger, "UTC offset out of range [-12, 14]: %d.", instruction->timezone.toOffset);
				valid = false;
			}
			break;
		}
		case INSTR_REPEAT: {
			if (*activeClock == NULL) {
				logError(_logger, "Operation 'repeat' used before declaring a clock.");
				valid = false;
			}
			if (instruction->repeat.times <= 0) {
				logError(_logger, "Repeat count must be greater than 0, got %d.", instruction->repeat.times);
				valid = false;
			}
			scopeStackPush(scopes, *activeClock);
			if (!_analyzeInstructionList(instruction->repeat.body, table, scopes, activeClock)) valid = false;
			scopeStackPop(scopes);
			break;
		}
		case INSTR_IF: {
			if (*activeClock == NULL) {
				logError(_logger, "Operation 'if' used before declaring a clock.");
				valid = false;
			}
			scopeStackPush(scopes, *activeClock);
			if (!_analyzeInstructionList(instruction->ifInstr.thenBranch, table, scopes, activeClock)) valid = false;
			scopeStackPop(scopes);
			if (instruction->ifInstr.elseBranch != NULL) {
				scopeStackPush(scopes, *activeClock);
				if (!_analyzeInstructionList(instruction->ifInstr.elseBranch, table, scopes, activeClock)) valid = false;
				scopeStackPop(scopes);
			}
			break;
		}
		case INSTR_ADD:
		case INSTR_SUB:
		case INSTR_ROUND:
		case INSTR_NEXT_HOUR:
		case INSTR_COLOR:
		case INSTR_BACKGROUND:
		case INSTR_BORDER:
		case INSTR_NUMBERS: {
			if (*activeClock == NULL) {
				logError(_logger, "Operation used before declaring a clock.");
				valid = false;
			}
			break;
		}
		case INSTR_RENDER:
			break;
	}
	return valid;
}

static bool _analyzeInstructionList(InstructionList * list, SymbolTable * table, ScopeStack * scopes, const char ** activeClock) {
	if (list == NULL) return true;
	bool headValid = _analyzeInstruction(list->instruction, table, scopes, activeClock);
	bool tailValid = _analyzeInstructionList(list->next, table, scopes, activeClock);
	return headValid && tailValid;
}

/* PUBLIC FUNCTIONS */

CompilationStatus analyzeSemantics(Program * program) {
	logDebugging(_logger, "Starting semantic analysis...");
	SymbolTable * table  = symbolTableCreate();
	ScopeStack * scopes  = scopeStackCreate();
	const char * activeClock = NULL;
	bool valid = _analyzeInstructionList(program->instructions, table, scopes, &activeClock);
	scopeStackDestroy(scopes);
	symbolTableDestroy(table);
	if (valid) {
		logDebugging(_logger, "Semantic analysis succeeded.");
		return SUCCEEDED;
	}
	logError(_logger, "Semantic analysis failed.");
	return FAILED;
}
