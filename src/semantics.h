/**
   @file semantics.h
   @brief Header file for classes and routines related to semantic checking
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   Contains the definitions of classes and functionality needed to perform
   semantic checking on a Crema program's AST
*/

#ifndef CREMA_SEMANTICS_H_
#define CREMA_SEMANTICS_H_

#include "decls.h"
#include "types.h"

/** 
 *  Stores the contextual information required to perform semantic analysis on a Crema program */
class SemanticContext {
public:
    int currScope; /**< Index to the current scope used for variable search */
    std::vector<VariableList*> vars; /**< Stack of scopes containing declared variables */
    std::vector<Type> currType; /**< List of return types for the stack of scopes */
    std::vector<NStructureDeclaration*> structs; /**< List of defined structures */
    FunctionList funcs; /**< List of defined functions */
    
    SemanticContext(); /**< Default constructor, creates the root (empty) scope */
    void newScope(Type & type);
    void delScope();
    NStructureDeclaration * searchStructs(NIdentifier & ident);
    NVariableDeclaration * searchVars(NIdentifier & ident);
    NFunctionDeclaration * searchFuncs(NIdentifier & ident);
    bool registerVar(NVariableDeclaration * var);
    bool registerFunc(NFunctionDeclaration * func);
    bool registerStruct(NStructureDeclaration * s);
};

#endif // CREMA_SEMANTICS_H_
