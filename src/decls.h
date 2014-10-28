/**
   @file decls.h
   @brief Header file for forward declarations
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>
*/

#ifndef CREMA_DECLS_H_
#define CREMA_DECLS_H_

#include <iostream>
#include <string>
#include <vector>

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
class Type;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;
typedef std::vector<NFunctionDeclaration*> FunctionList;
typedef std::vector<NValue*> ValueList;

extern SemanticContext rootCtx;

#endif // CREMA_DECLS_H_
