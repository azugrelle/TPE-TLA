%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

/**
 * @see https://westes.github.io/flex/manual/Definitions-Section.html#Definitions-Section
 */

extern CompilationStatus KeywordLexemeAction(TokenLabel label);
extern CompilationStatus OperatorLexemeAction(TokenLabel label);
extern CompilationStatus SymbolLexemeAction(TokenLabel label);
extern CompilationStatus IntegerLexemeAction();
extern CompilationStatus NameLexemeAction();
extern CompilationStatus EnterLineCommentLexemeAction(FlexContext context);
extern CompilationStatus LeaveLineCommentLexemeAction();
extern CompilationStatus EnterMultilineCommentLexemeAction(FlexContext context);
extern CompilationStatus LeaveMultilineCommentLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus UnknownLexemeAction();
extern CompilationStatus EOFLexemeAction();

%}

/**
 * Flex contexts (a.k.a. start conditions).
 *
 * @see https://westes.github.io/flex/manual/Start-Conditions.html
 */
%x LINE_COMMENT
%x MULTILINE_COMMENT

/**
 * Reusable patterns.
 *
 * @see https://westes.github.io/flex/manual/Matching.html
 * @see https://westes.github.io/flex/manual/Patterns.html
 */
whitespace			[[:space:]]+

%%

%{

/**
 * @see https://westes.github.io/flex/manual/Rules-Section.html#Rules-Section
 */

%}

"clock"								{ return KeywordLexemeAction(CLOCK); }
"add"								{ return KeywordLexemeAction(ADD); }
"sub"								{ return KeywordLexemeAction(SUB); }
"set"								{ return KeywordLexemeAction(SET); }
"hour"								{ return KeywordLexemeAction(HOUR); }
"minute"							{ return KeywordLexemeAction(MINUTE); }
"hours"								{ return KeywordLexemeAction(HOURS); }
"minutes"							{ return KeywordLexemeAction(MINUTES); }
"round"								{ return KeywordLexemeAction(ROUND); }
"to"								{ return KeywordLexemeAction(TO); }
"next"								{ return KeywordLexemeAction(NEXT); }
"repeat"							{ return KeywordLexemeAction(REPEAT); }
"if"								{ return KeywordLexemeAction(IF); }
"else"								{ return KeywordLexemeAction(ELSE); }
"color"								{ return KeywordLexemeAction(COLOR); }
"background"						{ return KeywordLexemeAction(BACKGROUND); }
"border"							{ return KeywordLexemeAction(BORDER); }
"numbers"							{ return KeywordLexemeAction(NUMBERS); }
"arabic"							{ return KeywordLexemeAction(ARABIC); }
"roman"								{ return KeywordLexemeAction(ROMAN); }
"black"								{ return KeywordLexemeAction(BLACK); }
"white"								{ return KeywordLexemeAction(WHITE); }
"green"								{ return KeywordLexemeAction(GREEN); }
"red"								{ return KeywordLexemeAction(RED); }
"blue"								{ return KeywordLexemeAction(BLUE); }
"AND"								{ return KeywordLexemeAction(AND); }
"OR"								{ return KeywordLexemeAction(OR); }
"NOT"								{ return KeywordLexemeAction(NOT); }
"UTC"								{ return KeywordLexemeAction(UTC); }

"->"								{ return OperatorLexemeAction(ARROW); }
"=="								{ return OperatorLexemeAction(EQ); }
"!="								{ return OperatorLexemeAction(NEQ); }
"<="								{ return OperatorLexemeAction(LTE); }
">="								{ return OperatorLexemeAction(GTE); }
"<"									{ return OperatorLexemeAction(LT); }
">"									{ return OperatorLexemeAction(GT); }
"+"									{ return OperatorLexemeAction(PLUS); }
"-"									{ return OperatorLexemeAction(MINUS); }
":"									{ return OperatorLexemeAction(COLON); }

"{"									{ return SymbolLexemeAction(OPEN_BRACE); }
"}"									{ return SymbolLexemeAction(CLOSE_BRACE); }
"("									{ return SymbolLexemeAction(OPEN_PARENTHESIS); }
")"									{ return SymbolLexemeAction(CLOSE_PARENTHESIS); }

[0-9]+								{ return IntegerLexemeAction(); }
[a-zA-Z][a-zA-Z0-9_]*				{ return NameLexemeAction(); }

"//"								{ return EnterLineCommentLexemeAction(LINE_COMMENT); }
<LINE_COMMENT>"\n"					{ return LeaveLineCommentLexemeAction(); }
<LINE_COMMENT>[^\n]+				{ return IgnoredLexemeAction(); }
<LINE_COMMENT>.						{ return IgnoredLexemeAction(); }

"/*"								{ return EnterMultilineCommentLexemeAction(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>"*/"				{ return LeaveMultilineCommentLexemeAction(); }
<MULTILINE_COMMENT>[^*]+			{ return IgnoredLexemeAction(); }
<MULTILINE_COMMENT>.				{ return IgnoredLexemeAction(); }

{whitespace}						{ return IgnoredLexemeAction(); }
.									{ return UnknownLexemeAction(); }

<<EOF>>								{ return EOFLexemeAction(); }

%%

/**
 * @see https://westes.github.io/flex/manual/User-Code-Section.html#User-Code-Section
 */

#include "FlexActions.h"
#include "FlexExport.h"
