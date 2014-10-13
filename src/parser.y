%{
    #include "ast.h"
    #include <stdlib.h>
    #include <stdio.h>

    NBlock *rootBlock;

    extern int yylex();
    void yyerror(const char *s) { fprintf(stderr, "ERROR: %s\n", s); }
%}

%define parse.error verbose

%union {
       NBlock *block;
       Node *node;
       int token;
       NIdentifier *ident;
       NExpression *expression;
       NStatement *statement;
       NVariableDeclaration *var_decl;
       std::vector<NVariableDeclaration*> *decl_args;
       std::vector<NExpression*> *call_args;
       std::string *string;
}

/* Terminal types */
%token <string> TIDENTIFIER TINT TDOUBLE
%token <token> TRETURN TSDEF TDEF TIF TELSE TFOREACH TAS TCEQ
%token <token> TCNEQ TCLE TCGE TCLT TCGT TTVOID TTINT TTUINT TTSTR TSTRUCT
%token <token> TTDOUBLE TEQUAL TLPAREN TRPAREN TLBRAC TRBRAC TMOD TPERIOD
%token <token> TMUL TADD TDIV TSUB TRBRACKET TLBRACKET TCOMMA TQUOTE

/* Non-terminal types */
%type <token> type comparison combine def
%type <ident> identifier
%type <statement> statement struct_decl var_decl func_decl assignment return loop conditional
%type <expression> expression value numeric list struct
%type <block> block program statements 
%type <call_args> func_call_arg_list
%type <decl_args> func_decl_arg_list var_decls

%left TADD TMUL TDIV
%left TMOD TSUB

%start program

%%

program : { rootBlock = NULL; } /* Empty program */
	| statements { rootBlock = $1; }
	;

statements : statement { $$ = new NBlock(); $$->statements.push_back($<statement>1); }
	   | statements statement { $1->statements.push_back($<statement>2); }
	   ;

statement : var_decl { }
	  | struct_decl { } 
	  | func_decl { }
	  | assignment { }
	  | conditional { }
	  | loop { }
	  | return { }
	  ;

conditional : TIF TLPAREN expression TRPAREN block TELSE conditional { $$ = new NIfStatement(*$3, *$5, $7); } /* else if */
	    | TIF TLPAREN expression TRPAREN block TELSE block { $$ = new NIfStatement(*$3, *$5, $7); } /* else */
	    | TIF TLPAREN expression TRPAREN block { $$ = new NIfStatement(*$3, *$5); } /* vanilla if */
	    ;

return : TRETURN expression { $$ = new NReturn(*$2); }
       ;

assignment : identifier TEQUAL expression { $$ = new NAssignmentStatement(*$1, *$3); }
	   ;

loop : TFOREACH TLPAREN identifier TAS identifier TRPAREN block { $$ = new NLoopStatement(*$3, *$5, *$7); }
     ;

struct_decl : TSTRUCT identifier TLBRACKET var_decls TRBRACKET { $$ = new NStructureDeclaration(*$2, *$4); }	
	    ;

var_decls : { $$ = new VariableList(); }
	  | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
	  | var_decls TCOMMA var_decl { $$->push_back($<var_decl>3); }
	  ;

var_decl : type identifier { $$ = new NVariableDeclaration($1, *$2); }
	 | type identifier TEQUAL expression { $$ = new NVariableDeclaration($1, *$2, $4); }
	 ;

func_decl : def type identifier TLPAREN func_decl_arg_list TRPAREN block { $$ = new NFunctionDeclaration($2, *$3, *$5, $7); }
	  ;

def : TDEF
    | TSDEF
    ;

block : TLBRACKET statements TRBRACKET { $$ = $2; }
      | TLBRACKET TRBRACKET { $$ = new NBlock(); }
      ;

func_decl_arg_list : /* Empty */ { $$ = new VariableList(); }
		   | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
		   | func_decl_arg_list TCOMMA var_decl { $$->push_back($<var_decl>3); }
		   ;

expression : expression combine expression { $$ = new NBinaryOperator(*$1, $2, *$3); }
	   | identifier TLPAREN func_call_arg_list TRPAREN { $$ = new NFunctionCall(*$1, *$3); }
	   | identifier { $$ = new NVariableAccess(*$1); }
	   | list { }
	   | struct { }
	   | TLPAREN expression TRPAREN { $$ = $2; }
	   | value { }
	   | expression comparison expression { $$ = new NBinaryOperator(*$1, $2, *$3); } 
	   | TSUB expression { NDouble *zero = new NDouble(0); $$ = new NBinaryOperator(*zero, $1, *$2); } /* negative numbers */
	   ;

func_call_arg_list : /* Empty */ { $$ = new ExpressionList(); }
		   | expression { $$ = new ExpressionList(); $$->push_back($<expression>1); }
		   | func_call_arg_list TCOMMA expression { $$->push_back($<expression>3); }
		   ;

numeric : TDOUBLE { $$ = new NDouble(atof($1->c_str())); $$->type = TTDOUBLE; delete $1; }
	| TINT { $$ = new NInt(atoi($1->c_str())); $$->type = TTINT; delete $1; }
	;

value : numeric { $$ = $1; }
      | TQUOTE TIDENTIFIER TQUOTE { std::string str = $2->c_str(); $$ = new NString(str); $$->type = TTSTR; delete $2; }
      ;

combine : TADD
	| TMUL
	| TMOD 
	| TSUB
	| TDIV
	;

list : identifier TLBRAC expression TRBRAC { $$ = new NListAccess(*$1, *$3); } /* Array access */
     ;

struct : identifier TPERIOD identifier { $$ = new NStructureAccess(*$1, *$3); } /* Structure access */
       ;

identifier : TIDENTIFIER { std::string str = $1->c_str(); $$ = new NIdentifier(str); delete $1; }
	   ;

comparison : TCEQ
	   | TCNEQ	
	   | TCGT
	   | TCLT
	   | TCGE
	   | TCLE
	   ;

type : TTDOUBLE
     | TTINT
     | TTUINT
     | TTSTR
     | TTVOID
     ;

%%
