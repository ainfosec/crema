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
bool NBlock::semanticAnalysis(SemanticContext * ctx) const
{
  // Points to the last element in the vector<int> currType.
  ctx->newScope(ctx->currType.back());
  for (int i = 0; i < statements.size(); i++)
    {
      if (!statements[i]->semanticAnalysis(ctx))
	return false;
    }
  ctx->delScope();
  return true;
}

bool NVariableDeclaration::semanticAnalysis(SemanticContext * ctx) const
{
  
}
