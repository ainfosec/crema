#include "ast.h"
#include "parser.h"

void SemanticContext::newScope()
{
  vars.push_back(new VariableList());
  funcs.push_back(new FunctionList());
  currScope++;
}

void SemanticContext::delScope()
{
  vars.pop_back();
  funcs.pop_back();
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
  for (int j = 0; j < (funcs[currScope])->size(); j++)
    {
      if (func->ident.value == funcs[currScope]->at(j)->ident.value)
	return false;
    }

  funcs[currScope]->push_back(func);
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
  for (int i = funcs.size() - 1; i >= 0; i--)
    {
      // Search through current scope for variable
      for (int j = 0; j < funcs[i]->size(); j++)
	{
	  if (ident.value == funcs[i]->at(j)->ident.value)
	    return funcs[i]->at(j);
	}
    }

  return NULL;
}

std::ostream & operator<<(std::ostream & os, const Node & node)
{
  node.print(os);
  return os;
}

std::ostream & NBlock::print(std::ostream & os) const
{
    os << "Block: {" << std::endl;
    for (int i = 0; i < statements.size(); i++)
    {
      os << *(statements[i]) << std::endl;
    }
    os << "}" << std::endl;
    return os;
}

bool NBlock::semanticAnalysis(SemanticContext * ctx) const
{
  ctx->newScope();
  for (int i = 0; i < statements.size(); i++)
    {
      if (!statements[i]->semanticAnalysis(ctx))
	return false;
    }
  ctx->delScope();
  return true;
}

std::ostream & NVariableDeclaration::print(std::ostream & os) const
{
  os << "Variable decl: " << type << " " << name;
  if (initializationExpression != NULL)
    os << " " << *initializationExpression;
  os << std::endl;
  return os;
}

std::ostream & NFunctionDeclaration::print(std::ostream & os) const
{
  os << "Function decl: " << type << " " << ident << "(";
  for (int i = 0; i < variables.size(); i++)
    {
      os << *(variables[i]) << " ";
    }
  os << ") " << *body << std::endl;
  
  return os;
}

std::ostream & NStructureDeclaration::print(std::ostream & os) const
{
  os << "Struct decl: " << ident << " {";
  for (int i = 0; i < members.size(); i++)
    {
      os << *(members[i]) << " ";
    }
  os << "}" << std::endl;
  
  return os;
}

std::ostream & NAssignmentStatement::print(std::ostream & os) const
{
  os << "Assignment: " << ident << " = " << expr << std::endl;
  return os;
}

std::ostream & NBinaryOperator::print(std::ostream & os) const
{
  os << "Binary Op: " << lhs << " " << op << " " << rhs << std::endl;
  return os;
}

std::ostream & NIdentifier::print(std::ostream & os) const
{
  os << "Identifier: " << value;
  return os;
}

std::ostream & NListAccess::print(std::ostream & os) const
{
  os << "List access: " << ident << "[" << index << "]";
  return os;
}

std::ostream & NStructureAccess::print(std::ostream & os) const
{
  os << "Struct access: " << ident << "." << member;
  return os;
}

std::ostream & NReturn::print(std::ostream & os) const
{
  os << "Return: " << retExpr << std::endl;
  return os;
}

std::ostream & NDouble::print(std::ostream & os) const
{
  os << "Value: " << value;
  return os;
}

std::ostream & NInt::print(std::ostream & os) const
{
  os << "Value: " << value;
  return os;
}

std::ostream & NValue::print(std::ostream & os) const
{
  os << "Generic NValue" << std::endl;
  return os;
}

std::ostream & NUInt::print(std::ostream & os) const
{
  os << "Value: " << value;
  return os;
}

std::ostream & NString::print(std::ostream & os) const
{
  os << "Value: " << value;
  return os;
}

std::ostream & NLoopStatement::print(std::ostream & os) const
{
  os << "Loop: " << list << " as " << asVar << std::endl;
  os << "{" << loopBlock << "}" << std::endl;
  return os;
}

std::ostream & NIfStatement::print(std::ostream & os) const
{
  if (elseblock == NULL && elseif == NULL)
    os << "If: (" << condition << ") then " << thenblock << std::endl;
  if (elseblock != NULL && elseif == NULL)
    os << "If: (" << condition << ") then: " << thenblock << " else: " << *(elseblock) << std::endl;
  if (elseblock == NULL && elseif != NULL)
    {
      os << "If: (" << condition << ") then " << thenblock << std::endl;
      os << "Else if: " << *(elseif) << std::endl;
    }
  if (elseblock != NULL && elseif != NULL)
    {
      os << "If: (" << condition << ") then " << thenblock << std::endl;
      os << "Else if: " << *(elseif) << " else: " << *(elseblock) << std::endl;
    }
  return os;
}

std::ostream & NFunctionCall::print(std::ostream & os) const
{
  os << "Function Call: " << ident << std::endl;
  for (int i = 0; i < args.size(); i++)
    {
      os << args[i] << std::endl;
    }
  return os;
}

std::ostream & NList::print(std::ostream & os) const
{
  os << "List: ";
    for (int i = 0; i < value.size(); i++)
    {
      os << value[i] << " ";
    }
    os << std::endl;
    return os;
}
