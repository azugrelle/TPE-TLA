%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"

/**
 * The error reporting function for Bison parser.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Error-Reporting-Function.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Tracking-Locations.html
 */
void yyerror(const YYLTYPE * location, const char * message) {}

%}

// You touch this, and you die.
%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
	/** Terminals. */

	signed int integer;
	TokenLabel token;

	/** Non-terminals */

	Program * program;
	InstructionList * instructionList;
	Instruction * instruction;
	ClockInstruction * clockInstruction;
	ArithmeticInstruction * arithmeticInstruction;
	SetInstruction * setInstruction;
	StyleInstruction * styleInstruction;
	TimezoneInstruction * timezoneInstruction;
	RepeatInstruction * repeatInstruction;
	IfInstruction * ifInstruction;
	Condition * condition;
	TimezoneExpr * timezoneExpr;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parsing succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
%destructor { destroyInstructionList($$); } <instructionList>
%destructor { destroyInstruction($$); }     <instruction>
%destructor { destroyCondition($$); }       <condition>
%destructor { destroyTimezoneExpr($$); }    <timezoneExpr>

/** Terminals – keywords. */
%token <token> CLOCK
%token <token> RENDER
%token <token> ADD
%token <token> SUB
%token <token> SET
%token <token> HOUR
%token <token> MINUTE
%token <token> HOURS
%token <token> MINUTES
%token <token> ROUND
%token <token> TO
%token <token> NEXT
%token <token> REPEAT
%token <token> IF
%token <token> ELSE
%token <token> COLOR
%token <token> BACKGROUND
%token <token> BORDER
%token <token> NUMBERS
%token <token> ARABIC
%token <token> ROMAN
%token <token> BLACK
%token <token> WHITE
%token <token> GREEN
%token <token> RED
%token <token> BLUE
%token <token> AND
%token <token> OR
%token <token> NOT
%token <token> UTC

/** Terminals – operators and symbols. */
%token <token> ARROW
%token <token> EQ
%token <token> NEQ
%token <token> LTE
%token <token> GTE
%token <token> LT
%token <token> GT
%token <token> PLUS
%token <token> MINUS
%token <token> COLON
%token <token> OPEN_BRACE
%token <token> CLOSE_BRACE
%token <token> OPEN_PARENTHESIS
%token <token> CLOSE_PARENTHESIS

/** Terminal – catch-all for unknown lexemes */
%token <token> UNKNOWN

/** Terminal – integer literal (carries its numeric value). */
%token <integer> INTEGER

/** Non-terminals. */
%type <program>         program
%type <instructionList> instructionList
%type <instruction>     instruction
%type <integer>         integer
%type <integer>         color
%type <integer>         numberType
%type <timezoneExpr>    timezoneExpr
%type <condition>       condition
%type <integer>         comparator

/**
 * Precedence and associativity.
 *
 * @see https://en.cppreference.com/w/cpp/language/operator_precedence.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
*/
%left OR
%left AND
%right NOT

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program:
	instructionList							{ $$ = ProgramSemanticAction($1); }
	;

instructionList:
	instruction instructionList				{ $$ = InstructionListSemanticAction($1, $2); }
	| %empty								{ $$ = EmptyInstructionListSemanticAction(); }
	;

instruction:
	CLOCK integer COLON integer				{ $$ = ClockSemanticAction($2, $4); }
	| RENDER								{ $$ = RenderSemanticAction(); }
	| ADD integer HOURS						{ $$ = AddSemanticAction($2, HOURS_UNIT); }
	| ADD integer MINUTES					{ $$ = AddSemanticAction($2, MINUTES_UNIT); }
	| SUB integer HOURS						{ $$ = SubSemanticAction($2, HOURS_UNIT); }
	| SUB integer MINUTES					{ $$ = SubSemanticAction($2, MINUTES_UNIT); }
	| SET HOUR integer						{ $$ = SetHourSemanticAction($3); }
	| SET MINUTE integer					{ $$ = SetMinuteSemanticAction($3); }
	| ROUND TO INTEGER MINUTES				{ $$ = RoundSemanticAction($3); if ($$ == NULL) YYABORT; }
	| NEXT HOUR								{ $$ = NextHourSemanticAction(); }
	| COLOR color							{ $$ = ColorSemanticAction($2); }
	| BACKGROUND color						{ $$ = BackgroundSemanticAction($2); }
	| BORDER color							{ $$ = BorderSemanticAction($2); }
	| NUMBERS numberType					{ $$ = NumbersSemanticAction($2); }
	| timezoneExpr ARROW timezoneExpr		{ $$ = TimezoneSemanticAction($1, $3); }
	| REPEAT INTEGER OPEN_BRACE instructionList CLOSE_BRACE
											{ $$ = RepeatSemanticAction($2, $4); }
	| IF OPEN_PARENTHESIS condition CLOSE_PARENTHESIS
	  OPEN_BRACE instructionList CLOSE_BRACE
											{ $$ = IfSemanticAction($3, $6, NULL); }
	| IF OPEN_PARENTHESIS condition CLOSE_PARENTHESIS
	  OPEN_BRACE instructionList CLOSE_BRACE
	  ELSE OPEN_BRACE instructionList CLOSE_BRACE
											{ $$ = IfSemanticAction($3, $6, $10); }
	;

integer:
	INTEGER									{ $$ = $1; }
	;

color:
	BLACK									{ $$ = COLOR_BLACK; }
	| WHITE									{ $$ = COLOR_WHITE; }
	| GREEN									{ $$ = COLOR_GREEN; }
	| RED									{ $$ = COLOR_RED; }
	| BLUE									{ $$ = COLOR_BLUE; }
	;

numberType:
	ARABIC									{ $$ = NUMBER_ARABIC; }
	| ROMAN									{ $$ = NUMBER_ROMAN; }
	;

timezoneExpr:
	UTC										{ $$ = TimezoneExprSemanticAction(0); }
	| UTC PLUS INTEGER						{ $$ = TimezoneExprSemanticAction($3); }
	| UTC MINUS INTEGER						{ $$ = TimezoneExprSemanticAction(-$3); }
	;

condition:
	HOUR comparator INTEGER					{ $$ = SimpleConditionSemanticAction(COMP_HOUR, $2, $3); }
	| MINUTE comparator INTEGER				{ $$ = SimpleConditionSemanticAction(COMP_MINUTE, $2, $3); }
	| condition AND condition				{ $$ = AndConditionSemanticAction($1, $3); }
	| condition OR condition				{ $$ = OrConditionSemanticAction($1, $3); }
	| NOT condition							{ $$ = NotConditionSemanticAction($2); }
	| OPEN_PARENTHESIS condition CLOSE_PARENTHESIS	{ $$ = $2; }
	;

comparator:
	EQ										{ $$ = CMP_EQ; }
	| NEQ									{ $$ = CMP_NEQ; }
	| LTE									{ $$ = CMP_LTE; }
	| GTE									{ $$ = CMP_GTE; }
	| LT									{ $$ = CMP_LT; }
	| GT									{ $$ = CMP_GT; }
	;

%%
