#ifdef AST_H_
#define AST_H_

#include <iostream>
#include <string>
#include <vector>
#include <llvm/Value.h>

class CodeGenContext;
class NExpression;
class NStatement;
class NVariableAssignment;
class NValue;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;
typedef std::vector<NValue*> ValueList;

class Node {
public:
    virtual ~Node() {}
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NExpression : public Node {
public:
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NStatement : public Node {
public:
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
}

class NIfStatement : public NStatement {
public:
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
}

class NValue : public NExpression {
public:
    virtual llvm::Value* codeGen(CodeGenContext & context) { }
};

class NDouble : public NValue {
public:
    double value;
    NNum(double value) : value(value) { }
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
