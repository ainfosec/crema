%{
    #include "ast.h"

    NBlock *rootBlock;

    extern int yylex();
    void yyerror(const char *s) { fprintf(stderr, "ERROR: %s\n", s); }
%}

%union {
       NBlock *block;
       Node *node;
       int token;
       NIdentifier *ident;
       NExpression *expression;
       NStatement *statement;
       std::vector<NVariableDeclaration*> *decl_args;
       std::vector<NExpression*> *call_args;
       std::string *string;
}

/* Terminal types */
%token <string> TIDENTIFIER TINT TDOUBLE
%token <token> TRETURN TSDEF TDEF TIF TELSE TFOREACH TAS TCEQ
%token <token> TCNEQ TCLE TCGE TCLT TCGT TTVOID TTINT TTUINT TTSTR
%token <token> TTDOUBLE TEQUAL TLPAREN TRPAREN TLBRAC TRBRAC TMOD
%token <token> TMUL TADD TDIV TSUB TRBRACKET TLBRACKET TCOMMA TQUOTE

/* Non-terminal types */
%type <token> type comparison numeric combine def
%type <ident> identifier
%type <statement> statement var_decl func_decl assignment return loop conditional
%type <expression> expression value
%type <block> block program statements 
%type <call_args> func_call_arg_list
%type <decl_args> func_decl_arg_list

%left TADD TMUL TDIV
%left TMOD TSUB

%start program

%%

program : statements { }
	;

statements : statement { }
	   | statements statement { }
	   ;

statement : var_decl { }
	  | func_decl { }
	  | expression { }
	  | assignment { }
	  | conditional { }
	  | loop { }
	  | return { }
	  ;

conditional : TIF TLPAREN expression TRPAREN block TELSE conditional { $$ = new NIfStatement(*$3, *$5, *$7); } /* else if */
	    | TIF TLPAREN expression TRPAREN block TELSE block { $$ = new NIfStatement(*$3, *$5, *$7); } /* else */
	    | TIF TLPAREN expression TRPAREN block { $$ = new NIfStatement(*$3, *$5); } /* vanilla if */
	    ;

return : TRETURN expression { }
       ;

assignment : identifier TEQUAL expression { }
	   ;

loop : TFOREACH TLPAREN identifier TAS identifier TRPAREN block { }
     ;

var_decl : type identifier { }
	 | type identifier TEQUAL expression { }
	 ;

func_decl : def type identifier TLPAREN func_decl_arg_list TRPAREN block { }
	  ;

def : TDEF
    | TSDEF
    ;

block : TLBRACKET statements TRBRACKET { }
      | TLBRACKET TRBRACKET { }
      ;

func_decl_arg_list : /* Empty */ { }
		   | var_decl { }
		   | func_decl_arg_list TCOMMA var_decl { }
		   ;

expression : expression combine expression { }
	   | identifier TLPAREN func_call_arg_list TRPAREN { }
	   | identifier { }
	   | TLPAREN expression TRPAREN { }
	   | value { }
	   | expression comparison expression { } 
	   | TSUB expression { } /* negative numbers */
	   ;

func_call_arg_list : /* Empty */ { }
		   | expression { }
		   | func_call_arg_list TCOMMA expression { }
		   ;

numeric : TDOUBLE { $$ = new NDouble(atof($1->c_str())); delete $1; }
	| TINT { $$ = new NInt(atoi($1->c_str())); delete $1; }
	;

value : numeric { $$ = $1; }
      | TQUOTE TIDENTIFIER TQUOTE { std::string str = $2->c_str(); $$ = new NString(str); delete $2; }
      ;

combine : TADD
	| TMUL
	| TMOD 
	| TSUB
	| TDIV
	;

identifier : TIDENTIFIER TLBRAC expression TRBRAC { } /* Array access */
	   | TIDENTIFIER { }
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
