#include "ast.h"
#include "parser.h"

/** 
 *  
 */
void SemanticContext::newScope(int type)
{
  vars.push_back(new VariableList());
  currType.push_back(type);
  currScope++;
}

void SemanticContext::delScope()
{
  vars.pop_back();
  currType.pop_back();
  currScope--;
}

bool SemanticContext::registerVar(NVariableDeclaration * var)
{
  // Search through current scope for variable duplication
  for (int j = 0; j < (vars[currScope])->size(); j++)
    {
      if (var->name.value == vars[currScope]->at(j)->name.value)
	return false;
    }

  vars[currScope]->push_back(var);
  return true;
}

bool SemanticContext::registerFunc(NFunctionDeclaration * func)
{
  // Search through current scope for function duplication
  for (int j = 0; j < funcs.size(); j++)
    {
      if (func->ident.value == funcs[j]->ident.value)
	return false;
    }

  funcs.push_back(func);
  return true;
}

/*
  Searches the local, then parent scopes for a variable declaration
 */
NVariableDeclaration * SemanticContext::searchVars(NIdentifier & ident) 
{
  // Search through stacks in reverse order
  for (int i = vars.size() - 1; i >= 0; i--)
    {
      // Search through current scope for variable
      for (int j = 0; j < (vars[i])->size(); j++)
	{
	  if (ident.value == vars[i]->at(j)->name.value)
	    return vars[i]->at(j);
	}
    }

  return NULL;
}

/*
  Searches the local, then parent scopes for a function declaration
 */
NFunctionDeclaration * SemanticContext::searchFuncs(NIdentifier & ident) 
{
  // Search through stacks in reverse order
  for (int i = 0; i < funcs.size(); i++)
    {
      if (ident.value == funcs[i]->ident.value)
	return funcs[i];
    }

  return NULL;
}

/** Iterates over the vector and returns 'false' if any of
 *  semanticAnalysis elements are false. */
bool NBlock::semanticAnalysis(SemanticContext * ctx)
{
  // Points to the last element in the vector<int> currType.
  ctx->newScope(ctx->currType.back());
  for (int i = 0; i < statements.size(); i++)
    {
      if (!((*(statements[i])).semanticAnalysis(ctx)))
	return false;
    }
  ctx->delScope();
  return true;
}

bool NBinaryOperator::semanticAnalysis(SemanticContext * ctx)
{
  if (lhs.getType(ctx) != rhs.getType(ctx))
    {
      std::cout << "Binary operator type mismatch for op: " << op << std::endl;
      return false;
    }
  return true;
}

bool NAssignmentStatement::semanticAnalysis(SemanticContext * ctx)
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (!var)
    {
      std::cout << "Assignment to undefined variable " << ident << std::endl;
      return false;
    }

  if (var->type != expr.getType(ctx))
    {
      std::cout << "Type mismatch for assignment to " << ident << std::endl;
      return false;
    }
  return true;
}

bool NReturn::semanticAnalysis(SemanticContext * ctx)
{
  if (retExpr.getType(ctx) != ctx->currType.back())
    {
      std::cout << "Returning type " << retExpr.getType(ctx) << " when a " << ctx->currType.back() << " was expected" << std::endl;
      return false;
    }
  return true;
}

int NVariableAccess::getType(SemanticContext * ctx) const
{
  NVariableDeclaration *var = ctx->searchVars(ident);
  if (var)
    {
      return var->type;
    }
  return 0;
}

int NFunctionCall::getType(SemanticContext * ctx) const 
{
  NFunctionDeclaration *func = ctx->searchFuncs(ident);
  if (func)
    {
      return func->type;
    }
  return 0;
}

bool NFunctionDeclaration::semanticAnalysis(SemanticContext * ctx)
{
  bool blockSA;
  if (!ctx->registerFunc(this)) 
    {
      std::cout << "Duplicate func decl for " << ident << std::endl;
      return false;
    } 
  ctx->newScope(type);
  for (int i = 0; i < variables.size(); i++)
    {
      ctx->registerVar(variables[i]);
    }
  blockSA = body->semanticAnalysis(ctx);
  ctx->delScope();
  return blockSA;
}

bool NVariableDeclaration::semanticAnalysis(SemanticContext * ctx)
{
  if (!ctx->registerVar(this)) 
    {
      std::cout << "Duplicate var decl for " << name << std::endl;
      // Variable collision
      return false;
    } 
  if (initializationExpression)
    {
      if (initializationExpression->getType(ctx) != type || !initializationExpression->semanticAnalysis(ctx))
	{
	  std::cout << "Type mismatch for " << name << std::endl;
	  return false;
	}
    }
  return true;
}


