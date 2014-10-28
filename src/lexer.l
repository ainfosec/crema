%{
#include <string>
#include "ast.h"
#include "parser.h"
#define SAVE_VAL yylval.string = new std::string(yytext, yyleng)
#define TOK(t) (yylval.token = (t))

extern "C" int yywrap() { }
%}

%option debug yylineno nodefault

%%

[ \t]			      ;
\n			      ; // TODO: Keep track of line count to provide more helpful error reporting
"return"		      return TOK(TRETURN);
"sdef"			      return TOK(TSDEF);
"def"		              return TOK(TDEF);
"struct"		      return TOK(TSTRUCT);
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
"true"			      return TOK(TTRUE);
"false"			      return TOK(TFALSE);
"bool"			      return TOK(TTBOOL);
"void"			      return TOK(TTVOID);
"int"			      return TOK(TTINT);
"uint"			      return TOK(TTUINT);
"string"		      return TOK(TTSTR);
"double"		      return TOK(TTDOUBLE);
[a-zA-Z_][a-zA-Z_0-9]* 	      SAVE_VAL; return TIDENTIFIER;
[0-9]+\.[0-9]+		      SAVE_VAL; return TDOUBLE;
[0-9]+			      SAVE_VAL; return TINT;
\"[^\"\n]*\"		      SAVE_VAL; return TSTRING;
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
"%"			      return TOK(TMOD);
"*"			      return TOK(TMUL);
"/"			      return TOK(TDIV);
"+"			      return TOK(TADD);
"-"			      return TOK(TSUB);
"{"			      return TOK(TLBRACKET);
"}"			      return TOK(TRBRACKET);
","			      return TOK(TCOMMA);
"\."			      return TOK(TPERIOD);
\#.*			      ; // Comment, ignore
.		printf("Unknown token!\n"); yyterminate();

%%