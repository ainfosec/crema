/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TIDENTIFIER = 258,
    TINT = 259,
    TDOUBLE = 260,
    TRETURN = 261,
    TSDEF = 262,
    TDEF = 263,
    TIF = 264,
    TELSE = 265,
    TFOREACH = 266,
    TAS = 267,
    TCEQ = 268,
    TCNEQ = 269,
    TCLE = 270,
    TCGE = 271,
    TCLT = 272,
    TCGT = 273,
    TTVOID = 274,
    TTINT = 275,
    TTUINT = 276,
    TTSTR = 277,
    TSTRUCT = 278,
    TTDOUBLE = 279,
    TEQUAL = 280,
    TLPAREN = 281,
    TRPAREN = 282,
    TLBRAC = 283,
    TRBRAC = 284,
    TMOD = 285,
    TPERIOD = 286,
    TMUL = 287,
    TADD = 288,
    TDIV = 289,
    TSUB = 290,
    TRBRACKET = 291,
    TLBRACKET = 292,
    TCOMMA = 293,
    TQUOTE = 294
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 14 "parser.y" /* yacc.c:1909  */

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

#line 107 "parser.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
