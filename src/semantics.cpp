/**
   @file semantics.cpp
   @brief Implementation for semantic analysis-related functionality of cremacc checking
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   This file contains the implementations for all functionality related to semantic analysis.
   This includes the SemanticContext implementation and the typing functions for the AST functions.
 */
#include "semantics.h"
#include "ast.h"
#include "parser.h"
#include "types.h"
#include <typeinfo>

/**
 * The root SemanticContext object to use when performing semantic analysis */
SemanticContext rootCtx;

SemanticContext::SemanticContext() 
{ 
  Type t;
  t.isList = false;
  t.typecode = INT;
  newScope(t); 
  currScope = 0; 
} 

/** 
    Creates a new scope for variable declarations. 
    'vars' is a std::vector<VariableList*> and VariableList is a 
    typedef std::vector<NVariableDeclaration*>. So when newScope is called,
    an empty vector of NVariableDeclarations is pushed to the back of the
    vars vector. 'currType' is a std::vector<Type>. The new type being pushed 
    to the back of currType contains a boolean 'isList' member and a
    TypeCodes 'typecode' variable, which is an enum containing all the possible
    types. currScope is incremented by one.

    @param type The return type of the scope (0 if void)
 */
void SemanticContext::newScope(Type & type)
{
  vars.push_back(new VariableList());
  currType.push_back(type);
  currScope++;
}

/**
   Deletes the most recent scope.
   Pops back the most recent VariableList (typedef std::vector<NVariableDeclaration*>)
   from the vars vector and the most recent Type object from the currType vector. 
   currScope is decremented by one.
*/
void SemanticContext::delScope()
{
  vars.pop_back();
  currType.pop_back();
  currScope--;
}

/**
   Registers the variable into the current scope and returns true or false depending
   on whether it was successfully added. The function looks to see if there is an 
   existing variable name (ident) in the current scope by iterating through the 
   VariableList (typedef std::vector<NVariableDeclaration*>) and comparing the 
   argument member, var->ident, with the other ident members within the VariableList.
   If there is no duplicate, the function will push_back the var class object to the
   vars vector.

   @param var Pointer to the NVariableDeclaration to add to the current scope
   @return true if the variable was added, false if it is a duplicate
*/
bool SemanticContext::registerVar(NVariableDeclaration * var)
{
    if (NULL != searchFuncs(var->ident))
    	return false;
  
  // Search through current scope for variable duplication
  for (auto it : (*(vars[currScope])))
    if ( var->ident == it->ident )
        return false;

  vars[currScope]->push_back(var);
  return true;
}

/**
   Registers the function into the global scope and returns true or false depending
   on whether it was successfully added. The function searches for the function name
   being passed as an argument (func->ident) matches with exising functions in
   the funcs vector. 

   @param func Pointer to the NFunctionDeclaration to add to the global scope
   @return true if the function was added, false if it is a duplicate
*/
bool SemanticContext::registerFunc(NFunctionDeclaration * func)
{
    if (NULL != searchVars(func->ident))
    	return false;

  // Search through for duplicate function duplication
  for (auto it : funcs)
      if ( func->ident == it->ident )
          return false;

  funcs.push_back(func);
  return true;
}

/**
   Registers the structure into the global scope and returns true or false depending 
   on whether it was successfully added. The function searches for struct name 
   being passed as an argument (s->ident) matches with existing struct names
   in the structs vector.

   @param s Pointer to the NStructureDeclaration to add to the global scope
   @return true if the structure was added, false if it is a duplicate
*/
bool SemanticContext::registerStruct(NStructureDeclaration * s)
{
  // Search through for duplicate struct duplication
  for (auto it : structs)
      if ( s->ident == it->ident )
          return false;

  structs.push_back(s);
  return true;
}

/**
  Searches the local, then parent scopes for a variable declaration. The search begins
  at the back of the std::vector<Variable*> vars vector. At each vars element the inner
  for loop looks for matches between the function argument ident and the ident member of the 
  NVariableDeclaration class. The function then returns a pointer to the class object of the
  referenced variable.

  @param ident NIdentifier to search for in the stack of scopes
  @return Pointer to NVariableDeclaration of the referenced variable or NULL if it cannot be found
 */
NVariableDeclaration * SemanticContext::searchVars(NIdentifier & ident) 
{
  // Search through stacks in reverse order
  for (int i = vars.size()-1; i >= 0; i--)
    {
      // Search through current scope for variable
      for (int j = 0; j < (vars[i])->size(); j++)
	{
	  if (ident == vars[i]->at(j)->ident)
	    return vars[i]->at(j);
	}
    }

  return NULL;
}

/**
  Searches for a function declaration. A std::vector<T>::iterator iterates over the funcs
  vector and the if statement compares the name of the argument ident with each function name
  funcs[index]->ident. 

  @param ident NIdentifier to search for in the global function scope
  @return Pointer to NFunctionDeclaration of the referenced function or NULL if it cannot be found
*/
NFunctionDeclaration * SemanticContext::searchFuncs(NIdentifier & ident) 
{
  for (auto it : funcs)
      if (ident == it->ident)
          return it;

  return NULL;
}


/**
  Searches for a structure declaration. A std::vector<T>::iterator moves over the structs
  vector and the if statement compares the name of the argument ident with each struct name
  struct[index]->ident.

  @param ident NIdentifier to search for in the global structure scope
  @return Pointer to NStructureDeclaration of the referenced structure or NULL if it cannot be found
*/
NStructureDeclaration * SemanticContext::searchStructs(NIdentifier & ident) 
{
  for (auto it : structs)
      if (ident == it->ident)
          return it;

  return NULL;
}

/**
   Iterates over the vector and returns 'false' if any of
   semanticAnalysis elements are false.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return true if the block passes semantic analysis, false otherwise
*/
bool NBlock::semanticAnalysis(SemanticContext * ctx)
{
  // Points to the last element in the vector<int> currType.
  ctx->newScope(ctx->currType.back());
  
  for( auto it : statements )
      if (!((*it).semanticAnalysis(ctx)))
          return false;

  ctx->delScope();
  return true;
}

/**
   Iterates over the vector and returns 'true' if any of
   checkRecursion elements are true.

   @param ctx Pointer to SemanticContext on which to perform the checks
   @param func Pointer to NFunctionDeclaration of the parent function that is being checked
   @return true if there is a recursive call, false otherwise
*/
bool NBlock::checkRecursion(SemanticContext *ctx, NFunctionDeclaration * func)
{
    for (auto it : statements)
        if ((*it).checkRecursion(ctx, func))
            return true;
    return false;
}

/**
   Checks the name of the function being called (ident) with the name of the member
   within the NFunctionCall object (func->ident), and returns true if the names match.
   Otherwise, other function names are searched within the body and checkRecursion is
   called again recursively.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @param func Pointer to the NFunctionDeclaration that is being checked
   @return true if there is a recursive call, false otherwise
*/
bool NFunctionCall::checkRecursion(SemanticContext * ctx, NFunctionDeclaration * func)
{
  if (func->ident == ident)
      return true;
  
  return ctx->searchFuncs(ident)->body->checkRecursion(ctx, func);
}

/**
   Performs the semantic analysis of a binary operator expression. This function compares
   the two types of the left- and right-hand-side of the expression by calling the function
   getType(ctx).   

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return true if lhs and rhs types mismatch, false if there is a type mismatch
*/
bool NBinaryOperator::semanticAnalysis(SemanticContext * ctx)
{
    Type & t1 = lhs.getType(ctx), & t2 = rhs.getType(ctx);
    if (!(t1 >= t2 || t2 >= t1))
    {
	std::cout << "Binary operator type mismatch for op: " << op << std::endl;
	return false;
    }
    return true;
}

/**
   This function creates two Type objects from the lhs and the rhs of a comparison
   statement. If the expression is not a comparison, the 'upcast' label compares the 
   types of the lhs and rhs and returns the type object with the higher precedence.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return Type object based on the types of the lhs and rhs sides of the expression.
*/
Type & NBinaryOperator::getType(SemanticContext * ctx) const
{
    Type & t1 = lhs.getType(ctx), & t2 = rhs.getType(ctx);
    switch (op)
    {
	// Comparisons return a BOOL
    case TCEQ:
    case TCNEQ:
    case TCLE:
    case TCLT:
    case TCGE:
    case TCGT:
    case TLOR:
    case TLAND:
	    return *(new Type(TTBOOL));
	    break;
	// Mathematical binary operations return the greater of the two types as long as they can be combined
    default:
	    break;
    }

    // Should consider alternate ways to implement this other than using the label, 'upcast'.
upcast:
    if (!(t1 >= t2 || t2 >= t1))
    {
	return *(new Type());
    }

    if (t1 == t2)
    {
	return t1;
    }

    return (t1 > t2) ? t1 : t2;
}

/**
   An NVariableDeclaration pointer, var, is created and assiged to
   the NVariableDeclaration object result of the searchVars function,
   which searches for the variable name (ident) within the context, ctx.
   If neither the variable name is not found or there is a type mismatch,
   semanticAnalysis returns false. True, otherwise.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return true if the variable and correct type are found, false otherwise.
*/
bool NAssignmentStatement::semanticAnalysis(SemanticContext * ctx)
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (!var)
  {
      std::cout << "Assignment to undefined variable " << ident << std::endl;
      return false;
  }
  
  if (var->type < expr.getType(ctx))
  {
      std::cout << "Type mismatch (" << var->type << " vs. " << expr.getType(ctx) << ") for assignment to " << ident << std::endl;
      return false;
  }
  if (var->type != expr.getType(ctx))
  {
      std::cout << "Warning: Upcast from " << var->type << " to " << expr.getType(ctx) << std::endl;
  }
  return true;
}

/** 
   An NVariableDeclaration pointer, var, is created and assigned to 
   the NVariableDeclaration object result of the searchVars function,
   which searches for the variable name (ident) within the context, ctx.
   If the assignment is to an undefined variable, the struct variable
   already exists, or if there is a type mismatch, semanticAnalysis will 
   return false. Otherwise, true. A warning will occur if the struture
   is upcast.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return true if the assignment passes semantic analysis, and false otherwise.
*/
bool NStructureAssignmentStatement::semanticAnalysis(SemanticContext * ctx)
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (!var)
  {
      std::cout << "Assignment to undefined variable " << ident << std::endl;
      return false;
  }
  if (!var->type.isStruct)
  {
      return false;
  }
  if (structure.getType(ctx) < expr.getType(ctx))
  {
      std::cout << "Type mismatch (" << structure.getType(ctx) << " vs. " << expr.getType(ctx) << ") for assignment to " << ident << std::endl;
      return false;
  }
  if (structure.getType(ctx) != expr.getType(ctx))
  {
      std::cout << "Warning: Upcast from " << structure.getType(ctx) << " to " << expr.getType(ctx) << std::endl;
  }
  return true;
}

/**
   An NVariableDeclaration pointer, var, is created and assigned to the 
   NVariableDeclaration object result of the searchVars funtion which
   searches for the variable name (ident) within the context, ctx.
   If the list being assigned contains and invalid name or invalid 
   mismathed types, the semantic analysis will fail. A warning will be
   given for upcast events related to the typing. 

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return true if list assignment passes semantic analaysis, and false otherwise.
*/
bool NListAssignmentStatement::semanticAnalysis(SemanticContext * ctx)
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (!var)
    {
      std::cout << "Assignment to undefined variable " << ident << std::endl;
      return false;
    }
  Type *t = new Type(var->type, false);
  if (*t < expr.getType(ctx))
  {
      std::cout << "Type mismatch (" << *t << " vs. " << expr.getType(ctx) << ") for assignment to " << ident << std::endl;
      return false;
  }
  if (*t != expr.getType(ctx))
  {
      std::cout << "Warning: Upcast from " << *t << " to " << expr.getType(ctx) << std::endl;
  }
  return true;
}

/**
   If the return expression type is further up the stack than the current 
   context type, this function will return false. True, otherwise.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return true if the type of the returned expression agrees with the current
   context at the back of the stack.
*/
bool NReturn::semanticAnalysis(SemanticContext * ctx)
{
  if (retExpr.getType(ctx) > ctx->currType.back())
  {
      std::cout << "Returning type " << retExpr.getType(ctx) << " when a " << ctx->currType.back() << " was expected" << std::endl;
      return false;
  }
  if (retExpr.getType(ctx) != ctx->currType.back())
  {
      std::cout << "Warning: Upcast from " << retExpr.getType(ctx) << " to " << ctx->currType.back() << std::endl;
  }

  return true;
}

/**
   This functions gets a type object from the semantic context of a list.
   First, a reference to a type object is created and either the Type() 
   constructor is called (if value.size() !> 0) or the reference is to
   the first index of the value vector. A pointer to this is then created.
   Then, the type is compared to the other types in the list and if a 
   different type is encoutered, a pointer to a new Type object is returned.
   Otherwise, the function returns the pointer *lt.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return pointer to a Type object.
*/
Type & NList::getType(SemanticContext * ctx) const
{
  Type & type = (value.size() > 0) ? value[0]->getType(ctx) : *(new Type());
  Type *lt = new Type(type, true);
  for (int i = 1; i < value.size(); i++)
    {
      if (value[i]->getType(ctx) != type)
	  return *(new Type());
    }

  return *lt;
}

/**
   This function checks whether the types within a list are the same. 

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return true if all types within a list match and false otherwise.
*/
bool NList::semanticAnalysis(SemanticContext * ctx)
{
  Type & type = (value.size() > 0) ? value[0]->getType(ctx) : *(new Type());
  for (int i = 1; i < value.size(); i++)
  {
      if (value[i]->getType(ctx) != type)
      {
	  std::cout << "List contains differing types!" << std::endl;
	  return false;
      }
  }
  
  return true;
}

/**
   Gets the type of a variable or creates a new Type() object if the
   searchVars(ident) function does not find the name of an existing 
   variable.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return Type object of the variable or a new Type object if it doesn't exist
*/
Type & NVariableAccess::getType(SemanticContext * ctx) const
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (var)
    {
      return var->type;
    }
  return *(new Type());
}

/**
   Checks whether the variable in the current semantic context exists, and
   returns the boolean result.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return true if the variable name is found in searchVars(ident) function, false otherwise
*/
bool NVariableAccess::semanticAnalysis(SemanticContext * ctx)
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (var)
    {
      return true;
    }
  return false;
}

/**
   Checks whether a list a present in the current semantic context.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return true if found, false if not found or if variable is not a list.
*/
bool NListAccess::semanticAnalysis(SemanticContext * ctx)
{
    NVariableDeclaration *var = ctx->searchVars(ident);
    if (var)
    {
	if (!var->type.isList)
	{
	    return false;
	}
	Type & t = index.getType(ctx);
	if (t.typecode != INT && t.typecode != UINT)
	{
	    return false;
	}
	return true;
    }
    return false;
    
}

/**
   Accesses a list element and returns the Type object of the element's type.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return A pointer to the list element's type or a pointer to a new Type object.
*/
Type & NListAccess::getType(SemanticContext * ctx) const
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (var)
  {
      Type *st = new Type(var->type, false);
      return *st;
  }
  return *(new Type());
}

/**
   Gets the Type object of an NFunctionCall object. If the context exists within
   the searchFuncs(ident), then a pointer to that type is returned. Otherwise,
   a new Type object is created and returned.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return Type object of the function call or new Type object is it doesn't already exist.
*/
Type & NFunctionCall::getType(SemanticContext * ctx) const 
{
  NFunctionDeclaration *func = ctx->searchFuncs(ident);
  if (func)
    {
      return func->type;
    }
  return *(new Type());
}

/**
   Does a semantic check for a function call and returns a boolean value. If there 
   is an invalid number of arguments or a type mismatch, the function returns false.
   True otherwise. A warning is given in the case of type upcasting.

   @param ctx Pointer to the SemanticContext on which to perform the checks
   @return true if the number of args and types agree, false otherwise
*/
bool NFunctionCall::semanticAnalysis(SemanticContext * ctx)
{
  NFunctionDeclaration *func = ctx->searchFuncs(ident);
  if (func)
  {
      if (func->variables.size() != args.size())
      {
	  std::cout << "Call to " << ident << " with invalid number of arguments! " << func->variables.size() << " expected, " << args.size() << " provided" << std::endl;
	  return false;
      }
      for (int i = 0; i < args.size(); i++)
      {
	  if (args[i]->getType(ctx) > func->variables[i]->type)
	  {
	      std::cout << "Type mismatch when calling function: " << ident << std::endl;
	      return false;
	  }
	  if (args[i]->getType(ctx) != func->variables[i]->type)
	  {
	      std::cout << "Warning: Type upcast for argument: " << ident << std::endl;
	  }
      }
      return true;
  }
  std::cout << "Call to undefined function: " << ident << std::endl;
  return false;
}

/**
   Performs a semantic check for a loop statement. If the list variable is not defined 
   or the variable is not a list, the function returns false. Otherwise, semantic 
   analysis is perform on the loopBlock, delScope() method is called and the result 
   of the semantic analysis on the block is returned.

   @param ctx Pointer to the SemanticContext on which to perform checks.
   @return true if the semantic analysis on the block returns true, false if undefined or not a list. 
*/
bool NLoopStatement::semanticAnalysis(SemanticContext * ctx)
{
    NVariableDeclaration *l = ctx->searchVars(list);
    bool blockSA;
    Type *st;
    
    if (NULL == l)
    {
	std::cout << "List variable " << list << " not defined!" << std::endl;
	return false;
    }
    if (!l->type.isList)
    {
	std::cout << "Variable " << list << " not a list!" << std::endl;
	return false;
    }
    ctx->newScope(ctx->currType.back());
    st = new Type(l->type, false);
    
    ctx->registerVar(new NVariableDeclaration(*st, asVar));
    
    blockSA = loopBlock.semanticAnalysis(ctx);
    ctx->delScope();
    return blockSA;
}

/**
   Checks a function declaration for recursion and semantic analysis.

   @param ctx Pointer to the SemanticContext on which to perform checks.
   @return true if the function passes semantic analysis with no recursion, false otherwise
*/
bool NFunctionDeclaration::semanticAnalysis(SemanticContext * ctx)
{
  bool blockSA, blockRecur;
  ctx->newScope(type);
  for (auto it : variables)
  {
      if (!ctx->registerVar(it))
      {
          ctx->delScope();
          return false;
      }
  }

  blockSA = body->semanticAnalysis(ctx);
  blockRecur = body->checkRecursion(ctx, this);
  if (blockRecur)
    {
      std::cout << "Recursive function call in " << ident << std::endl;
    }
  ctx->delScope();
  return (blockSA && !blockRecur);
}

/**
   Performs semantic analysis on an if statement and returns the boolean results.
   If the conditional statement cannot evaluate to a boolean, or if any of the code
   blocks in the if, else, elseif blocks is false, then the semantic analysis will
   return false. Function returns true if all checks pass.

   @param ctx Pointer to the SemanticContext on which to perform checks.
   @return true if semantic analysis is passed, false if conditional statement or blocks do no pass.
*/
bool NIfStatement::semanticAnalysis(SemanticContext * ctx)
{
    Type & condType = condition.getType(ctx);
    if (condType.typecode == STRING || condType.typecode == INVALID || condType.typecode == VOID)
    {
	std::cout << "Condition cannot evaluate to a boolean!" << std::endl;
	return false;
    }
    if (elseblock && !elseblock->semanticAnalysis(ctx))
    {
	return false;
    }
    if (elseif && !elseif->semanticAnalysis(ctx))
    {
	return false;
    }
    return thenblock.semanticAnalysis(ctx);
}

/**
   Performs semantic analysis on a variable declaration by checking existing struct and variable
   names. Returns the boolean result of the check. 

   @param ctx Pointer to the SemanticContext on which to perform checks.
   @return true if semantic analysis is passed, false if variables already exist or assignment type does not match.
*/
bool NVariableDeclaration::semanticAnalysis(SemanticContext * ctx)
{
    if (type.isStruct)
    {
	StructType *st = (StructType *) &type;
	NStructureDeclaration *sd = ctx->searchStructs(st->ident);
	if (NULL == sd)
	{
	    std::cout << "Declaring variable of undefined struct type: " << st->ident << std::endl;
	    return false;
	}
    }
    
    if (!ctx->registerVar(this)) 
    {
	std::cout << "Duplicate var decl for " << ident << std::endl;
	// Variable collision
	return false;
    } 
    if (initializationExpression)
    {
	if (type < initializationExpression->getType(ctx))
	{
	    std::cout << "Type mismatch for " << ident << " (" << type << " vs. " << initializationExpression->getType(ctx) << ")" << std::endl;
	    return false;
	}
	return initializationExpression->semanticAnalysis(ctx);
    }
    return true;
}

/**
   Performs semantic analysis on structure declaration by iterating over the members
   to make sure there are no duplicates.  

   @param ctx Pointer to the SemanticContext on which to perform checks.
   @return true if semantic analysis is passed, false if duplicate var names exist 
*/
bool NStructureDeclaration::semanticAnalysis(SemanticContext * ctx)
{
    ctx->newScope(*(new Type()));
    for (auto it : members)
    {
    	if (!ctx->registerVar(it))
    	{
	    std::cout << "Duplicate struct member declaration for struct " << ident << std::endl;
	    ctx->delScope();
	    return false;
    	}  
    }
    
    ctx->delScope();
    return true;
}

/**
   Gets the Type object from a structure's member. If a varaible name is not found 
   or if it does not have isStruct set to true, a new Type() object is returned. The
   for loop iterates over the members, finds the name, and returns its Type object.
   Default is a new Type() object.

   @param ctx Pointer to the SemanticContext on which to perform checks.
   @return Type object of struct member or new Type() object if not found.
*/
Type & NStructureAccess::getType(SemanticContext * ctx) const
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (!var)
  {
      return *(new Type());
  }
  if (!(var->type.isStruct))
  {
      return *(new Type());
  }
  StructType *st = (StructType *) &(var->type);
  NStructureDeclaration *sd = ctx->searchStructs(st->ident);
  if (!sd)
  {
      return *(new Type());
  }

  for (auto it : sd->members)
     if (member == it->ident)
        return it->type;

  return *(new Type());
}

/**
   Performs semantic analysis check on structure member. If the struct member name 
   cannot be found or if the struct name itself cannot be found, semanticAnalysis
   will return false. True otherwise.

   @param ctx Pointer to the SemanticContext on which to perform checks.
   @return true if semantic analysis passes, false if members or struct are not found.
*/
bool NStructureAccess::semanticAnalysis(SemanticContext * ctx)
{
    NVariableDeclaration * var = ctx->searchVars(ident);
    if (NULL == var)
    {
	std::cout << "Structure variable " << ident << " cannot be found!" << std::endl;
	return false;
    }
    StructType *st = (StructType *) &(var->type);
    NStructureDeclaration * s = ctx->searchStructs(st->ident);
    if (NULL == s)
    {
	std::cout << "Reference to undefined structure " << st->ident << std::endl;
	return false;
    }

    for (auto it : s->members)
        if (it->ident == member)
            return true;
    
    std::cout << "Reference to non-existent member " << member << " of structure variable " << ident << std::endl;
    return false;
}
