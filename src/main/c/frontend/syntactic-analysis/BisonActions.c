#include "BisonActions.h"

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PUBLIC FUNCTIONS */

Program * ProgramSemanticAction(InstructionList * instructions) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->instructions = instructions;
	_compilerState->abstractSyntaxtTree = program;
	return program;
}

InstructionList * InstructionListSemanticAction(Instruction * instruction, InstructionList * next) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	InstructionList * list = calloc(1, sizeof(InstructionList));
	list->instruction = instruction;
	list->next = next;
	return list;
}

InstructionList * EmptyInstructionListSemanticAction(void) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

Instruction * ClockSemanticAction(int hour, int minute) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_CLOCK;
	instruction->clock.hour = hour;
	instruction->clock.minute = minute;
	return instruction;
}

Instruction * RenderSemanticAction(void) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_RENDER;
	return instruction;
}

Instruction * AddSemanticAction(int value, TimeUnit unit) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_ADD;
	instruction->arithmetic.value = value;
	instruction->arithmetic.unit = unit;
	return instruction;
}

Instruction * SubSemanticAction(int value, TimeUnit unit) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_SUB;
	instruction->arithmetic.value = value;
	instruction->arithmetic.unit = unit;
	return instruction;
}

Instruction * SetHourSemanticAction(int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_SET_HOUR;
	instruction->setTime.value = value;
	return instruction;
}

Instruction * SetMinuteSemanticAction(int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_SET_MINUTE;
	instruction->setTime.value = value;
	return instruction;
}

Instruction * RoundSemanticAction(void) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_ROUND;
	return instruction;
}

Instruction * NextHourSemanticAction(void) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_NEXT_HOUR;
	return instruction;
}

Instruction * ColorSemanticAction(Color color) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_COLOR;
	instruction->style.color = color;
	return instruction;
}

Instruction * BackgroundSemanticAction(Color color) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_BACKGROUND;
	instruction->style.color = color;
	return instruction;
}

Instruction * BorderSemanticAction(Color color) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_BORDER;
	instruction->style.color = color;
	return instruction;
}

Instruction * NumbersSemanticAction(NumberType numberType) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_NUMBERS;
	instruction->numbers.type = numberType;
	return instruction;
}

int TimezoneExprSemanticAction(int offset) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return offset;
}

Instruction * TimezoneSemanticAction(int fromOffset, int toOffset) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_TIMEZONE;
	instruction->timezone.fromOffset = fromOffset;
	instruction->timezone.toOffset = toOffset;
	return instruction;
}

Instruction * RepeatSemanticAction(int times, InstructionList * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_REPEAT;
	instruction->repeat.times = times;
	instruction->repeat.body = body;
	return instruction;
}

Instruction * IfSemanticAction(Condition * condition, InstructionList * thenBranch, InstructionList * elseBranch) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instruction * instruction = calloc(1, sizeof(Instruction));
	instruction->type = INSTR_IF;
	instruction->ifInstr.condition = condition;
	instruction->ifInstr.thenBranch = thenBranch;
	instruction->ifInstr.elseBranch = elseBranch;
	return instruction;
}

Condition * SimpleConditionSemanticAction(Component component, Comparator comparator, int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Condition * condition = calloc(1, sizeof(Condition));
	condition->type = COND_SIMPLE;
	condition->simple.component = component;
	condition->simple.comparator = comparator;
	condition->simple.value = value;
	return condition;
}

Condition * AndConditionSemanticAction(Condition * left, Condition * right) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Condition * condition = calloc(1, sizeof(Condition));
	condition->type = COND_AND;
	condition->binary.left = left;
	condition->binary.right = right;
	return condition;
}

Condition * OrConditionSemanticAction(Condition * left, Condition * right) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Condition * condition = calloc(1, sizeof(Condition));
	condition->type = COND_OR;
	condition->binary.left = left;
	condition->binary.right = right;
	return condition;
}

Condition * NotConditionSemanticAction(Condition * operand) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Condition * condition = calloc(1, sizeof(Condition));
	condition->type = COND_NOT;
	condition->unary.operand = operand;
	return condition;
}
