#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

/* Program */
Program * ProgramSemanticAction(InstructionList * instructions);

/* Instruction list */
InstructionList * InstructionListSemanticAction(Instruction * instruction, InstructionList * next);
InstructionList * EmptyInstructionListSemanticAction(void);

/* Instructions */
Instruction * ClockSemanticAction(int hour, int minute);
Instruction * RenderSemanticAction(void);
Instruction * AddSemanticAction(int value, TimeUnit unit);
Instruction * SubSemanticAction(int value, TimeUnit unit);
Instruction * SetHourSemanticAction(int value);
Instruction * SetMinuteSemanticAction(int value);
Instruction * RoundSemanticAction(void);
Instruction * NextHourSemanticAction(void);
Instruction * ColorSemanticAction(Color color);
Instruction * BackgroundSemanticAction(Color color);
Instruction * BorderSemanticAction(Color color);
Instruction * NumbersSemanticAction(NumberType numberType);
TimezoneExpr * TimezoneExprSemanticAction(int offset);
Instruction * TimezoneSemanticAction(TimezoneExpr * from, TimezoneExpr * to);
Instruction * RepeatSemanticAction(int times, InstructionList * body);
Instruction * IfSemanticAction(Condition * condition, InstructionList * thenBranch, InstructionList * elseBranch);

/* Conditions */
Condition * SimpleConditionSemanticAction(Component component, Comparator comparator, int value);
Condition * AndConditionSemanticAction(Condition * left, Condition * right);
Condition * OrConditionSemanticAction(Condition * left, Condition * right);
Condition * NotConditionSemanticAction(Condition * operand);

#endif
