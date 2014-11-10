%{
    #include "decls.h"
    #include "ast.h"
    #include "semantics.h"
    #include <stdlib.h>
    #include <stdio.h>

    NBlock *rootBlock;

    extern int yylex();
    void yyerror(const char *s) { fprintf(stderr, "ERROR: %s\n", s); exit(-1); }
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
       int fn; /* which function */
}

/* Terminal types */
%token <string> TIDENTIFIER TINT TDOUBLE TSTRING
%token <token> TRETURN TSDEF TDEF TIF TELSE TFOREACH TAS TCEQ
%token <token> TCNEQ TCLE TCGE TCLT TCGT TTVOID TTINT TTUINT TTSTR TSTRUCT
%token <token> TTDOUBLE TEQUAL TLPAREN TRPAREN TLBRAC TRBRAC TMOD TPERIOD
%token <token> TMUL TADD TDIV TSUB TRBRACKET TLBRACKET TCOMMA TTBOOL TTRUE TFALSE
%token <token> TAND TNOT TOR TUMINUS TUPLUS

/* Non-terminal types */
%type <token> type /* comparison combine */ def
%type <ident> identifier
%type <statement> statement struct_decl var_decl list_decl func_decl assignment return loop conditional
%type <expression> expression value numeric list_access struct list var_access factor term
%type <block> block program statements 
%type <call_args> func_call_arg_list
%type <decl_args> func_decl_arg_list var_decls


%left TADD TSUB
%left TMOD TMUL TDIV

%start program

%%

program : { rootBlock = NULL; } /* Empty program */
	| statements { rootBlock = $1; }
	;

statements : statement { $$ = new NBlock(); $$->statements.push_back($<statement>1); }
	   | statements statement { $1->statements.push_back($<statement>2); }
	   ;

statement : var_decl { }
	  | struct_decl { if(!rootCtx.registerStruct((NStructureDeclaration *) $1)) yyerror("Duplicate struct declaration!"); $$ = $1; }
	  | func_decl { if(!rootCtx.registerFunc((NFunctionDeclaration *) $1)) yyerror("Duplicate function declaration!"); $$ = $1; }
	  | assignment { }
	  | list_decl { }
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
	   | list_access TEQUAL expression { $$ = new NListAssignmentStatement(((NListAccess *) $1)->ident, *((NListAccess *) $1), *$3); }
	   | struct TEQUAL expression { $$ = new NStructureAssignmentStatement(((NStructureAccess *) $1)->ident, *((NStructureAccess *) $1), *$3); }
	   ;

loop : TFOREACH TLPAREN identifier TAS identifier TRPAREN block { $$ = new NLoopStatement(*$3, *$5, *$7); }
     ;

struct_decl : TSTRUCT identifier TLBRACKET var_decls TRBRACKET { $$ = new NStructureDeclaration(*$2, *$4); }	
	    ;

list_decl : type identifier TLBRAC TRBRAC { Type *t = new Type($1, true); $$ = new NVariableDeclaration(*t, *$2); }
	  | type identifier TLBRAC TRBRAC TEQUAL expression { Type *t = new Type($1, true); $$ = new NVariableDeclaration(*t, *$2, $6); }
	  ;

var_decls : { $$ = new VariableList(); }
	  | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
	  | var_decls TCOMMA var_decl { $$->push_back($<var_decl>3); }
	  ;

var_decl : type identifier { $$ = new NVariableDeclaration(*(new Type($1)), *$2); }
	 | type identifier TEQUAL expression { $$ = new NVariableDeclaration(*(new Type($1)), *$2, $4); }
	 ;

func_decl : def type identifier TLPAREN func_decl_arg_list TRPAREN block { $$ = new NFunctionDeclaration(*(new Type($2)), *$3, *$5, $7); }
	  | def type TLBRAC TRBRAC identifier TLPAREN func_decl_arg_list TRPAREN block { $$ = new NFunctionDeclaration(*(new Type($2, true)), *$5, *$7, $9); }
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

expression : expression TADD term { $$ = new NBinaryOperator(*$1, $2, *$3); }
           | expression TSUB term { $$ = new NBinaryOperator(*$1, $2, *$3); }
           | term { }
           ;

term : term TMUL factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | term TDIV factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | term TMOD factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | term TAND factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | term TOR factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | term TCEQ factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | term TCNEQ factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | term TCGT factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | term TCLT factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | term TCGE factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | term TCLE factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | factor { }
     ;

factor : var_access { }
       | list { }
       | value { }
       | identifier TLPAREN func_call_arg_list TRPAREN { $$ = new NFunctionCall(*$1, *$3); }
       | TLPAREN expression TRPAREN { $$ = $2; }
       ;

/*
expression : expression TADD expression { $$ = new NBinaryOperator(*$1, $2, *$3); } 
           | expression TSUB expression { $$ = new NBinaryOperator(*$1, $2, *$3); }
           | expression TMUL expression { $$ = new NBinaryOperator(*$1, $2, *$3); }
           | expression TDIV expression { $$ = new NBinaryOperator(*$1, $2, *$3); }
           | expression TMOD expression { $$ = new NBinaryOperator(*$1, $2, *$3); }
           | expression TAND expression { $$ = new NBinaryOperator(*$1, $2, *$3); }
           | expression TOR expression { $$ = new NBinaryOperator(*$1, $2, *$3); }
	       | expression comparison expression { $$ = new NBinaryOperator(*$1, $2, *$3); } 
           | var_access { }
           | var_access combine expression { $$ = new NBinaryOperator(*$1, $2, *$3); }
           | var_access comparison expression { $$ = new NBinaryOperator(*$1, $2, *$3); }
           | identifier TLPAREN func_call_arg_list TRPAREN { $$ = new NFunctionCall(*$1, *$3); }
           | list { }
           | value { } 
           | TNOT expression { $$ = new NBinaryOperator(*$2, $1, *$2); } 
	       | TLPAREN expression TRPAREN { $$ = $2; } 
           ;
*/

var_access : identifier { $$ = new NVariableAccess(*$1); }
	   | list_access { }
	   | struct { }
	   ;

list : TLBRAC func_call_arg_list TRBRAC { $$ = new NList(*$2); }
     ;

func_call_arg_list : /* Empty */ { $$ = new ExpressionList(); }
		   | expression { $$ = new ExpressionList(); $$->push_back($<expression>1); }
		   | func_call_arg_list TCOMMA expression { $$->push_back($<expression>3); }
		   ;

numeric : TDOUBLE { $$ = new NDouble(atof($1->c_str())); $$->type = *(new Type(TTDOUBLE)); delete $1; }
	| TSUB TDOUBLE { NDouble *zero = new NDouble(0); zero->type = *(new Type(TTDOUBLE)); NDouble *d = new NDouble(atof($2->c_str())); d->type = Type(TTDOUBLE); delete $2; $$ = new NBinaryOperator(*zero, $1, *d); } 
	| TINT { $$ = new NInt(atoi($1->c_str())); $$->type = *(new Type(TTINT)); delete $1; }
	| TSUB TINT { NInt *zero = new NInt(0); zero->type = TTINT; NInt *i = new NInt(atoi($2->c_str())); i->type = *(new Type(TTINT)); delete $2; $$ = new NBinaryOperator(*zero, $1, *i); } 
	;

value : numeric { $$ = $1; }
      | TSTRING { std::string str = $1->c_str(); $$ = new NString(str); $$->type = *(new Type(TTSTR)); delete $1; }
      | TTRUE { $$ = new NBool(true); $$->type = *(new Type(TTBOOL)); }
      | TFALSE { $$ = new NBool(false); $$->type = *(new Type(TTBOOL)); }
      ;

/*
combine : TADD
	| TMUL
	| TMOD 
	| TSUB
	| TDIV
	| TOR
	| TAND
	;
*/

list_access : identifier TLBRAC expression TRBRAC { $$ = new NListAccess(*$1, *$3); } /* Array access */
     	    ;

struct : identifier TPERIOD identifier { $$ = new NStructureAccess(*$1, *$3); } /* Structure access */
       ;

identifier : TIDENTIFIER { std::string str = $1->c_str(); $$ = new NIdentifier(str); delete $1; }
	   ;

/*
comparison : TCEQ
	   | TCNEQ	
	   | TCGT
	   | TCLT
	   | TCGE
	   | TCLE
       ;
*/

type : TTDOUBLE
     | TTINT
     | TTUINT
     | TTSTR
     | TTVOID
     | TTBOOL
     ;

%%
