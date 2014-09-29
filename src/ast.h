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
class NValue;
class NBlock;
class NIdentifier;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;
typedef std::vector<NValue*> ValueList;

class Node {
public:
    virtual ~Node() { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NBlock : public NExpression {
public:
    StatementList statements;
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
    friend std::ostream & operator<<(std::ostream & os, const NBlock & block);
};

class NAssignmentStatement : public NStatement {
public:
    NIdentifier & ident;
    NExpression * expr;
NAssignmentStatement(NIdentifier & ident, NExpression * expr) : ident(ident), expr(expr) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
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
};

class NBinaryOperator : public NExpression {
public:
    int op;
    NExpression & lhs;
    NExpression & rhs;
NBinaryOperator(NExpression & lhs, int op, NExpression & rhs) : lhs(lhs), op(op), rhs(rhs) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NVariableDeclaration : public NStatement {
public:
    int & type;
    NIdentifier & name;
    NExpression *initializationExpression;
NVariableDeclaration(int & type, NIdentifier & name) : type(type), name(name) { }
NVariableDeclaration(int & type, NIdentifier & name, NExpression *initExpr) : type(type), name(name), initializationExpression(initExpr) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NFunctionDeclaration : public NStatement {
public:
    VariableList variables;
    NIdentifier & ident;
    int type;
    NBlock *body;
NFunctionDeclaration(int type, NIdentifier & ident, VariableList & variables, NBlock *body) : type(type), ident(ident), variables(variables), body(body) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NReturn : public NStatement {
public:
    NExpression *retExpr;
NReturn(NExpression *re) : retExpr(re) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NValue : public NExpression {
public:
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NDouble : public NValue {
public:
    double value;
NDouble(double value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NUInt : public NValue {
public:
    unsigned long int value;
NUInt(unsigned long int value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NInt : public NValue {
public:
    long int value;
NInt(long int value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NString : public NValue {
public:
    std::string value;
NString(const std::string & value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NList : public NValue {
public:
    ValueList value;
    NList() {}
NList(ValueList & list) : value(list) { }
};

class NIdentifier : public Node {
public:
    std::string value;
NIdentifier(const std::string & value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};



#endif // AST_H_
