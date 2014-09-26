%{
#include <string>
#include "ast.h"
#include "parser.h"
#define SAVE_VAL yyval.value = new std::string(yytext, yyleng)  
#define TOK(t) (yyval.token = (t))
%}

%option debug

%%

"return"		      return TOK(TRETURN);
"sdef"			      return TOK(TSDEF);
"def"		              return TOK(TDEF);
"if"			      return TOK(TIF);
"else"		              return TOK(TELSE);
"foreach"		      return TOK(TFOREACH);
"as"			      return TOK(TAS);
"eq"			      return TOK(TCEQ);
"neq"			      return TOK(TCNEQ);
"le"			      return TOK(TCLE);
"ge"			      return TOK(TCGE);
"gt"			      return TOK(TCGT);
"lt"			      return TOK(TCLT);
"void"			      return TOK(TTVOID);
"int"			      return TOK(TTINT);
"uint"			      return TOK(TTUINT);
"string"		      return TOK(TTSTR);
"double"		      return TOK(TTDOUBLE);
[a-zA-Z_][a-zA-Z_0-9]* 	      SAVE_VAL; return TIDENTIFIER;
[0-9]+\.[0-9]+		      SAVE_VAL; return TDOUBLE;
[0-9]			      SAVE_VAL; return TINT;
"="			      return TOK(TEQUAL);
"=="			      return TOK(TCEQ);
"!="			      return TOK(TCNEQ);
"<"			      return TOK(TCLT);
"<="			      return TOK(TCLE);
">"			      return TOK(TCGT);
">="			      return TOK(TCGE);
"("			      return TOK(TLPAREN);
")"			      return TOK(TRPAREN);
"["			      return TOK(TLBRAC);
"]"			      return TOK(TRBRAC);
"\""			      return TOK(TQUOTE);
"%"			      return TOK(TMOD);
"*"			      return TOK(TMUL);
"/"			      return TOK(TDIV);
"+"			      return TOK(TADD);
"-"			      return TOK(TSUB);
"{"			      return TOK(TLBRACKET);
"}"			      return TOK(TRBRACKET);
","			      return TOK(TCOMMA);
[ \t]		;
\n		; // TODO: Keep track of line count to provide more helpful error reporting
.		printf("Unknown token!\n"); yyterminate();

%%