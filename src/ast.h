/**
   @file ast.h
   @brief Header file storing the AST class definitions
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   Stores the class definitions for all AST-related classes and the definitions for
   walking the AST for semantic analysis. All AST classes are derived from Node
*/

#ifndef CREMA_AST_H_
#define CREMA_AST_H_

#include <iostream>
#include <string>
#include <vector>
#include <llvm/IR/Value.h>
#include "types.h"

class CodeGenContext;
class NExpression;
class NStatement;
class NVariableAssignment;
class NVariableDeclaration;
class NStructureDeclaration;
class NFunctionDeclaration;
class NValue;
class NBlock;
class NListAccess;
class NStructureAccess;
class NIdentifier;
class SemanticContext;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;
typedef std::vector<NFunctionDeclaration*> FunctionList;
typedef std::vector<NValue*> ValueList;

/** 
 *  Stores the contextual information required to perform semantic analysis on a Crema program */
class SemanticContext {
public:
    int currScope; /**< Index to the current scope used for variable search */
    std::vector<VariableList*> vars; /**< Stack of scopes containing declared variables */
    std::vector<int> currType; /**< List of return types for the stack of scopes */
    std::vector<NStructureDeclaration*> structs; /**< List of defined structures */
    FunctionList funcs; /**< List of defined functions */
    
    SemanticContext() { newScope(0); currScope = 0; } /**< Default constructor, creates the root (empty) scope */
    void newScope(int type);
    void delScope();
    NStructureDeclaration * searchStructs(NIdentifier & ident);
    NVariableDeclaration * searchVars(NIdentifier & ident);
    NFunctionDeclaration * searchFuncs(NIdentifier & ident);
    bool registerVar(NVariableDeclaration * var);
    bool registerFunc(NFunctionDeclaration * func);
    bool registerStruct(NStructureDeclaration * s);
};

extern SemanticContext rootCtx;

/** 
 *  The base class containing all the language constructs. */
class Node {
 public:
  virtual ~Node() { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  virtual std::ostream & print(std::ostream & os) const { };
  virtual bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration *func) { };
  virtual bool semanticAnalysis(SemanticContext *ctx) { };
  friend std::ostream & operator<<(std::ostream & os, const Node & node);  
};

/**
 *  An expression is something that evaluates to a value. 
 *  (Ex. 45*3 % 20)  */
class NExpression : public Node {
public:
    int type; /**< Expression type used for type-checking during semantic analysis */
    virtual int getType(SemanticContext * ctx) const { }
};

/**
 *  A statement is a line of code that does something. In other words,
 *  you can't pass a statement as a parameter. 
 *  (Ex. if (x==1) print("yes") else print("no")) */
class NStatement : public Node {
};

/**
 *  One or more statements that evaluate to an NExpression value. */
class NBlock : public Node {
public:
    StatementList statements; /**< Vector of statements in the NBlock */
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
    bool semanticAnalysis(SemanticContext *ctx);
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration *func);
};

/**
 *  Statement assigning an expression to a variable */
class NAssignmentStatement : public NStatement {
public:
    NIdentifier & ident; /**< Variable identifier */
    NExpression & expr; /**< Expression to assign to variable */
NAssignmentStatement(NIdentifier & ident, NExpression & expr) : ident(ident), expr(expr) { } /**< Default constructor to create an NAssignmentStatement with the passed identifier and expression */
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    bool semanticAnalysis(SemanticContext * ctx);
    std::ostream & print(std::ostream & os) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return expr.checkRecursion(ctx, func); }
};

/**
 *  Statement assigning an expression to a list element */
class NListAssignmentStatement : public NAssignmentStatement {
public:
    NListAccess & list; /**< NListAccess for storing information about list index */
NListAssignmentStatement(NIdentifier & ident, NListAccess & list, NExpression & expr) : NAssignmentStatement(ident, expr), list(list) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return expr.checkRecursion(ctx, func); }
};

/**
 *  Statement assigning an expression to a struct member */
class NStructureAssignmentStatement : public NAssignmentStatement {
public:
    NStructureAccess & structure; /**< NStructureAccess for storing information about struct member(s) */
NStructureAssignmentStatement(NIdentifier & ident, NStructureAccess & structure, NExpression & expr) : NAssignmentStatement(ident, expr), structure(structure) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return expr.checkRecursion(ctx, func); }
};

/**
 *  Looping construct */
class NLoopStatement : public NStatement {
public:
    NIdentifier & list; /**< List variable name to loop through */
    NIdentifier & asVar; /**< Temporary variable name inside of loop block to reference current list element */
    NBlock & loopBlock; /**< NBlock to execute in the loop */
NLoopStatement(NIdentifier & list, NIdentifier & asVar, NBlock & loopBlock) : list(list), asVar(asVar), loopBlock(loopBlock) { }
    std::ostream & print(std::ostream & os) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return loopBlock.checkRecursion(ctx, func); }
};

/**
 *  If statement (can include elseif and else) */
class NIfStatement : public NStatement {
public:
    NExpression & condition; /**< Expression to evaluate to determine if the conditional will execute */
    NBlock & thenblock; /**< NBlock to execute if the condition is true */
    NBlock * elseblock; /**< Optional pointer to NBlock to execute if the condition is false */
    NStatement * elseif; /**< Optional pointer to NBlock to execute if the elseif condition is true */
NIfStatement(NExpression & condition, NBlock & thenblock, NBlock * elseblock) : condition(condition), thenblock(thenblock), elseblock(elseblock), elseif(NULL) { }
NIfStatement(NExpression & condition, NBlock & thenblock, NStatement * elseif) : condition(condition), thenblock(thenblock), elseblock(NULL), elseif(elseif) { }
NIfStatement(NExpression & condition, NBlock & thenblock) : condition(condition), thenblock(thenblock), elseblock(NULL), elseif(NULL) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return thenblock.checkRecursion(ctx, func) || (elseblock ? elseblock->checkRecursion(ctx, func) : false) || (elseif ? elseif->checkRecursion(ctx, func) : false); }
};

/**
 *  Binary operator expression (e.g. lhs + rhs) */
class NBinaryOperator : public NExpression {
public:
    int op; /**< Binary operator to execute on the lhs and rhs */
    NExpression & lhs; /**< Left hand-side of binary operator */
    NExpression & rhs; /**< Right hand-side of binary operator */
NBinaryOperator(NExpression & lhs, int op, NExpression & rhs) : lhs(lhs), op(op), rhs(rhs) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    bool semanticAnalysis(SemanticContext *ctx);
    int getType(SemanticContext * ctx) const { return lhs.getType(ctx) == rhs.getType(ctx) ? lhs.getType(ctx) : 0; }
    std::ostream & print(std::ostream & os) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return lhs.checkRecursion(ctx, func) || rhs.checkRecursion(ctx, func); }
};

/**
 *  Declaring a variable and its type, optionally with an expression for its initial value */
class NVariableDeclaration : public NStatement {
public:
    int type; /**< Type of variable */
    NIdentifier & ident; /**< Name of variable */
    NExpression *initializationExpression; /**< Pointer to optional initialization expression */
    int size; /**< Size (1 for singleton, >1 for list) */
NVariableDeclaration(int type, NIdentifier & name) : type(type), ident(name), initializationExpression(NULL), size(1) { }
NVariableDeclaration(int type, NIdentifier & name, NExpression *initExpr) : type(type), ident(name), size(1), initializationExpression(initExpr) { }
NVariableDeclaration(int type, NIdentifier & name, int size) : type(type), ident(name), initializationExpression(NULL), size(size) { }
NVariableDeclaration(int type, NIdentifier & name, int size, NExpression *initExpr) : type(type), ident(name), initializationExpression(initExpr), size(size) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
    bool semanticAnalysis(SemanticContext *ctx);
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return initializationExpression ? initializationExpression->checkRecursion(ctx, func) : false; }
};

/**
 *  Declares a function */
class NFunctionDeclaration : public NStatement {
public:
    VariableList variables; /**< List of variables to take as arguments */
    NIdentifier & ident; /**< Name of function */
    int type; /**< Return type of function */
    bool listReturn; /**< If this function returns a list of types or is a singleton */
    NBlock *body; /**< NBlock body of function */
NFunctionDeclaration(int type, NIdentifier & ident, VariableList & variables, NBlock *body) : type(type), ident(ident), variables(variables), body(body), listReturn(false) { }
NFunctionDeclaration(int type, NIdentifier & ident, bool listReturn, VariableList & variables, NBlock *body) : type(type), ident(ident), variables(variables), body(body), listReturn(listReturn) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    bool semanticAnalysis(SemanticContext * ctx);
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return false; }
    std::ostream & print(std::ostream & os) const;
};

/**
 *  Declares a structure */
class NStructureDeclaration : public NStatement {
public:
    VariableList members; /**< List of variable members of structure */
    NIdentifier & ident; /**< Name of structure */
NStructureDeclaration(NIdentifier & ident, VariableList & members) : ident(ident), members(members) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return false; }
};

/**
 *  Expression resolving to a function call */
class NFunctionCall : public NExpression {
public:
    ExpressionList args;  /**< List of function call arguments */
    NIdentifier & ident; /**< Name of function to call */
NFunctionCall(NIdentifier & ident, ExpressionList & args) : ident(ident), args(args) { }
    int getType(SemanticContext * ctx) const;
    std::ostream & print(std::ostream & os) const;
    bool semanticAnalysis(SemanticContext * ctx);
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func);
};

/**
 *  Expression resolving to a structure access */
class NStructureAccess : public NExpression {
public:
    NIdentifier & member; /**< Structure member name to access */
    NIdentifier & ident; /**< Name of structure variable */
NStructureAccess(NIdentifier & ident, NIdentifier & member) : ident(ident), member(member) { }
    std::ostream & print(std::ostream & os) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return false; }
};

/**
 *  Expression resolving to list access */
class NListAccess : public NExpression {
public:
    NExpression & index; /**< NExpression that resolves to list index */
    NIdentifier & ident; /**< Name of list variable to access */
NListAccess(NIdentifier & ident, NExpression & index) : ident(ident), index(index) { }
    std::ostream & print(std::ostream & os) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return false; }
};

/**
 *  Expression resolving to a singleton variable access */
class NVariableAccess : public NExpression {
public:
    NIdentifier & ident; /**< Name or variable to access */
NVariableAccess(NIdentifier & ident) : ident(ident) { }
    std::ostream & print(std::ostream & os) const;
    int getType(SemanticContext * ctx) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return false; }
};

/**
 *  Return statement */
class NReturn : public NStatement {
public:
    NExpression & retExpr; /**< NExpression to return to parent scope */
NReturn(NExpression & re) : retExpr(re) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    bool semanticAnalysis(SemanticContext * ctx);
    std::ostream & print(std::ostream & os) const;
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return retExpr.checkRecursion(ctx, func); }
};

/**
 *  Base class for values (ints, strings, doubles, lists, etc.) */
class NValue : public NExpression {
public:
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
    int getType(SemanticContext * ctx) const { return type; }
    bool semanticAnalysis(SemanticContext * ctx) { return true; }
    bool checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func) { return false; }
};

/**
 *  Double */
class NDouble : public NValue {
public:
    double value; /**< Value of double */
NDouble(double value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
};

/**
 *  Unsigned int */
class NUInt : public NValue {
public:
    unsigned long int value; /**< Value of unsigned int */
NUInt(unsigned long int value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
};

/**
 *  Signed int */
class NInt : public NValue {
public:
    long int value; /**< Value of signed int */
NInt(long int value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
};

/**
 * String type */
class NString : public NValue {
public:
    std::string value; /**< Value ot string */
NString(const std::string & value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
};

/**
 *  List type */
class NList : public NValue {
public:
    ExpressionList value; /**< List of NExpression elements */
    NList() { } /**< Default constructor to initialize an empty list */
NList(ExpressionList & list) : value(list) { }
    int getType(SemanticContext * ctx) const;
    std::ostream & print(std::ostream & os) const;
};

/**
 * Identifier */
class NIdentifier : public Node {
public:
    std::string value; /**< Identifier value */
NIdentifier(const std::string & value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    std::ostream & print(std::ostream & os) const;
    friend bool operator==(const NIdentifier & i1, const NIdentifier & i2);
};



#endif // CREMA_AST_H_
