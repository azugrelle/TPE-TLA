#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

ModuleDestructor initializeAbstractSyntaxTreeModule();


typedef enum { HOURS_UNIT, MINUTES_UNIT } TimeUnit;
typedef enum { COLOR_BLACK, COLOR_WHITE, COLOR_GREEN, COLOR_RED, COLOR_BLUE } Color;
typedef enum { NUMBER_ARABIC, NUMBER_ROMAN } NumberType;
typedef enum { COMP_HOUR, COMP_MINUTE } Component;
typedef enum { CMP_EQ, CMP_NEQ, CMP_LTE, CMP_GTE, CMP_LT, CMP_GT } Comparator;
typedef enum { COND_SIMPLE, COND_AND, COND_OR, COND_NOT } ConditionType;
typedef enum {
	INSTR_CLOCK, INSTR_RENDER, INSTR_ADD, INSTR_SUB,
	INSTR_SET_HOUR, INSTR_SET_MINUTE, INSTR_ROUND, INSTR_NEXT_HOUR,
	INSTR_COLOR, INSTR_BACKGROUND, INSTR_BORDER, INSTR_NUMBERS,
	INSTR_TIMEZONE, INSTR_REPEAT, INSTR_IF
} InstructionType;

typedef struct InstructionList InstructionList;
typedef struct Instruction Instruction;
typedef struct Condition Condition;
typedef struct TimezoneExpr TimezoneExpr;
typedef struct Program Program;

typedef Instruction ClockInstruction;
typedef Instruction ArithmeticInstruction;
typedef Instruction SetInstruction;
typedef Instruction StyleInstruction;
typedef Instruction TimezoneInstruction;
typedef Instruction RepeatInstruction;
typedef Instruction IfInstruction;


struct Condition {
	ConditionType type;
	union {
		struct { Component component; Comparator comparator; int value; } simple;
		struct { Condition * left; Condition * right; } binary;
		struct { Condition * operand; } unary;
	};
};

struct TimezoneExpr {
	int offset;
};

struct InstructionList {
	Instruction * instruction;
	InstructionList * next;
};

struct Instruction {
	InstructionType type;
	union {
		struct { char * name; int hour; int minute; } clock;
		struct { int value; TimeUnit unit; } arithmetic;
		struct { int value; } setTime;
		struct { Color color; } style;
		struct { NumberType type; } numbers;
		struct { int fromOffset; int toOffset; } timezone;
		struct { int times; InstructionList * body; } repeat;
		struct {
			Condition * condition;
			InstructionList * thenBranch;
			InstructionList * elseBranch;
		} ifInstr;
	};
};

struct Program {
	InstructionList * instructions;
};

void destroyCondition(Condition * condition);
void destroyTimezoneExpr(TimezoneExpr * expr);
void destroyInstruction(Instruction * instruction);
void destroyInstructionList(InstructionList * list);
void destroyProgram(Program * program);

#endif
