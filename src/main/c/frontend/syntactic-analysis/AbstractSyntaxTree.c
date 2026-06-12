#include "AbstractSyntaxTree.h"

static Logger * _logger = NULL;

static void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

void destroyCondition(Condition * condition) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (condition == NULL) return;
	switch (condition->type) {
		case COND_AND:
		case COND_OR:
			destroyCondition(condition->binary.left);
			destroyCondition(condition->binary.right);
			break;
		case COND_NOT:
			destroyCondition(condition->unary.operand);
			break;
		case COND_SIMPLE:
			break;
	}
	free(condition);
}

void destroyTimezoneExpr(TimezoneExpr * expr) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (expr == NULL) return;
	free(expr);
}

void destroyInstruction(Instruction * instruction) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (instruction == NULL) return;
	switch (instruction->type) {
		case INSTR_CLOCK:
			free(instruction->clock.name);
			break;
		case INSTR_REPEAT:
			destroyInstructionList(instruction->repeat.body);
			break;
		case INSTR_IF:
			destroyCondition(instruction->ifInstr.condition);
			destroyInstructionList(instruction->ifInstr.thenBranch);
			destroyInstructionList(instruction->ifInstr.elseBranch);
			break;
		default:
			break;
	}
	free(instruction);
}

void destroyInstructionList(InstructionList * list) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (list == NULL) return;
	destroyInstruction(list->instruction);
	destroyInstructionList(list->next);
	free(list);
}

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program == NULL) return;
	destroyInstructionList(program->instructions);
	free(program);
}
