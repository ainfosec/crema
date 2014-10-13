#ifndef AST_H_
#define AST_H_

#include <iostream>
#include <string>
#include <vector>
#include <llvm/IR/Value.h>

class CodeGenContext;
class NExpression;
class NStatement;
class NVariableAssignment;
class NVariableDeclaration;
class NFunctionDeclaration;
class NValue;
class NBlock;
class NIdentifier;
class SemanticContext;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;
typedef std::vector<NFunctionDeclaration*> FunctionList;
typedef std::vector<NValue*> ValueList;

/** 
 *  Defines the context for a node. */
class SemanticContext {
 public:
  int currScope;
  std::vector<VariableList*> vars;
  std::vector<int> currType;
  FunctionList funcs;
  
  SemanticContext() { newScope(0); currScope = 0; }  
  void newScope(int type);
  void delScope();
  NVariableDeclaration * searchVars(NIdentifier & ident);
  NFunctionDeclaration * searchFuncs(NIdentifier & ident);
  bool registerVar(NVariableDeclaration * var);
  bool registerFunc(NFunctionDeclaration * func);
};

/** 
 *  The base class containing all the language constructs. */
class Node {
 public:
  virtual ~Node() { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  virtual std::ostream & print(std::ostream & os) const { };
  virtual bool semanticAnalysis(SemanticContext *ctx) { };
  friend std::ostream & operator<<(std::ostream & os, const Node & node);  
};

/**
 *  An expression is something that evaluates to a value. 
 *  (Ex. 45*3 % 20)  */
class NExpression : public Node {
 public:
  int type;
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
  StatementList statements;
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
  bool semanticAnalysis(SemanticContext *ctx);
};

class NAssignmentStatement : public NStatement {
 public:
  NIdentifier & ident;
  NExpression & expr;
 NAssignmentStatement(NIdentifier & ident, NExpression & expr) : ident(ident), expr(expr) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  bool semanticAnalysis(SemanticContext * ctx);
  std::ostream & print(std::ostream & os) const;
};

class NLoopStatement : public NStatement {
 public:
  NIdentifier & list;
  NIdentifier & asVar;
  NBlock & loopBlock;
 NLoopStatement(NIdentifier & list, NIdentifier & asVar, NBlock & loopBlock) : list(list), asVar(asVar), loopBlock(loopBlock) { }
  std::ostream & print(std::ostream & os) const;
};

class NIfStatement : public NStatement {
 public:
  NExpression & condition;
  NBlock & thenblock;
  NBlock * elseblock;
  NStatement * elseif;
 NIfStatement(NExpression & condition, NBlock & thenblock, NBlock * elseblock) : condition(condition), thenblock(thenblock), elseblock(elseblock), elseif(NULL) { }
 NIfStatement(NExpression & condition, NBlock & thenblock, NStatement * elseif) : condition(condition), thenblock(thenblock), elseblock(NULL), elseif(elseif) { }
 NIfStatement(NExpression & condition, NBlock & thenblock) : condition(condition), thenblock(thenblock), elseblock(NULL), elseif(NULL) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};

class NBinaryOperator : public NExpression {
 public:
  int op;
  NExpression & lhs;
  NExpression & rhs;
 NBinaryOperator(NExpression & lhs, int op, NExpression & rhs) : lhs(lhs), op(op), rhs(rhs) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};

class NVariableDeclaration : public NStatement {
 public:
  int type;
  NIdentifier & name;
  NExpression *initializationExpression;
 NVariableDeclaration(int type, NIdentifier & name) : type(type), name(name), initializationExpression(NULL) { }
 NVariableDeclaration(int type, NIdentifier & name, NExpression *initExpr) : type(type), name(name), initializationExpression(initExpr) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
  bool semanticAnalysis(SemanticContext *ctx);
};

class NFunctionDeclaration : public NStatement {
 public:
  VariableList variables;
  NIdentifier & ident;
  int type;
  NBlock *body;
 NFunctionDeclaration(int type, NIdentifier & ident, VariableList & variables, NBlock *body) : type(type), ident(ident), variables(variables), body(body) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};

class NStructureDeclaration : public NStatement {
 public:
  VariableList members;
  NIdentifier & ident;
 NStructureDeclaration(NIdentifier & ident, VariableList & members) : ident(ident), members(members) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};

class NFunctionCall : public NExpression {
 public:
  ExpressionList args;
  NIdentifier & ident;
 NFunctionCall(NIdentifier & ident, ExpressionList & args) : ident(ident), args(args) { }
  std::ostream & print(std::ostream & os) const;
};

class NStructureAccess : public NExpression {
 public:
  NIdentifier & member;
  NIdentifier & ident;
 NStructureAccess(NIdentifier & ident, NIdentifier & member) : ident(ident), member(member) { }
  std::ostream & print(std::ostream & os) const;
};

class NListAccess : public NExpression {
 public:
  NExpression & index;
  NIdentifier & ident;
 NListAccess(NIdentifier & ident, NExpression & index) : ident(ident), index(index) { }
  std::ostream & print(std::ostream & os) const;
};

class NReturn : public NStatement {
 public:
  NExpression & retExpr;
 NReturn(NExpression & re) : retExpr(re) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};

class NValue : public NExpression {
 public:
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};

class NDouble : public NValue {
 public:
  double value;
 NDouble(double value) : value(value) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};

class NUInt : public NValue {
 public:
  unsigned long int value;
 NUInt(unsigned long int value) : value(value) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};

class NInt : public NValue {
 public:
  long int value;
 NInt(long int value) : value(value) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};

class NString : public NValue {
 public:
  std::string value;
 NString(const std::string & value) : value(value) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};

class NList : public NValue {
 public:
  ValueList value;
  NList() {}
 NList(ValueList & list) : value(list) { }
  std::ostream & print(std::ostream & os) const;
};

class NIdentifier : public Node {
 public:
  std::string value;
 NIdentifier(const std::string & value) : value(value) { }
  virtual llvm::Value* codeGen(CodeGenContext & context) { }
  std::ostream & print(std::ostream & os) const;
};



#endif // AST_H_
