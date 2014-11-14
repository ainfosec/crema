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
  //for (auto it = statements.begin(); it != statements.end(); ++it)
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

bool NFunctionCall::checkRecursion(SemanticContext * ctx, NFunctionDeclaration * func)
{
  if (func->ident == ident)
    {
      return true;
    }
  
  return ctx->searchFuncs(ident)->body->checkRecursion(ctx, func);
}

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
    case TOR:
    case TAND:
	return *(new Type(TTBOOL));
	break;
	// Mathematical binary operations return the greater of the two types as long as they can be combined
    default:
	break;
    }

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

bool NStructureAssignmentStatement::semanticAnalysis(SemanticContext * ctx)
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (!var)
    {
      std::cout << "Assignment to undefined variable " << ident << std::endl;
      return false;
    }
  if (!var->type.structt)
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

Type & NVariableAccess::getType(SemanticContext * ctx) const
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (var)
    {
      return var->type;
    }
  return *(new Type());
}

bool NVariableAccess::semanticAnalysis(SemanticContext * ctx)
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (var)
    {
      return true;
    }
  return false;
}

bool NListAccess::semanticAnalysis(SemanticContext * ctx)
{
    NVariableDeclaration *var = ctx->searchVars(ident);
    if (var)
    {
	if (!var->type.list)
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

Type & NFunctionCall::getType(SemanticContext * ctx) const 
{
  NFunctionDeclaration *func = ctx->searchFuncs(ident);
  if (func)
    {
      return func->type;
    }
  return *(new Type());
}

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

bool NFunctionDeclaration::semanticAnalysis(SemanticContext * ctx)
{
  bool blockSA, blockRecur;
  ctx->newScope(type);
  for (int i = 0; i < variables.size(); i++)
    {
	if (!ctx->registerVar(variables[i]))
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

bool NVariableDeclaration::semanticAnalysis(SemanticContext * ctx)
{
    if (type.structt)
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

bool NStructureDeclaration::semanticAnalysis(SemanticContext * ctx)
{
    ctx->newScope(*(new Type()));
    for (int i = 0; i < members.size(); i++)
    {
	if (!ctx->registerVar(members[i]))
	{
	    std::cout << "Duplicate struct member declaration for struct " << ident << std::endl;
	    ctx->delScope();
	    return false;
	}
    }
    ctx->delScope();
    return true;
}

Type & NStructureAccess::getType(SemanticContext * ctx) const
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (!var)
  {
      return *(new Type());
  }
  if (!(var->type.structt))
  {
      return *(new Type());
  }
  StructType *st = (StructType *) &(var->type);
  NStructureDeclaration *sd = ctx->searchStructs(st->ident);
  if (!sd)
  {
      return *(new Type());
  }
  for (int i = 0; i < sd->members.size(); i++)
  {
      if (member == sd->members[i]->ident)
      {
	  return sd->members[i]->type;
      }
  }
  return *(new Type());
}

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
    for (int i = 0; i < s->members.size(); i++)
    {
	if (s->members[i]->ident == member)
	{
	    return true;
	}
    }
    std::cout << "Reference to non-existent member " << member << " of structure variable " << ident << std::endl;
    return false;
}
