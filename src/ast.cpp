/**
   @file ast.cpp
   @brief Implementation file for AST-related classes
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   Stores the implementations for the AST classes. Includes pretty-printing logic
   and some operator overloads for AST manipulation and analysis.
*/
#include "ast.h"
#include "parser.h"

/**
   Overload for the == operator to allow for simple comparison of two NIdentifiers

   @param i1 First NIdentifier to compare
   @param i2 Second NIdentifier to compare
   @return true if the Identifiers resolve to the same value, false otherwise
*/
bool operator==(const NIdentifier & i1, const NIdentifier & i2)
{
  return i1.value == i2.value;
}

/**
   Overload for the << operator to allow pretty printing of AST objects and AST as a whole

   @param os Output stream to print to
   @param node Node to pretty-print
   @return Output stream passed in
*/
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

std::ostream & NVariableDeclaration::print(std::ostream & os) const
{
  if (!type.list)
  {
      os << "Variable declared --- (" << type << " " << ident << ")";
  }
  else
  {
      os << "List declared --- (" << type << " " << ident << "[])";
  }
  if (initializationExpression != NULL)
  {
      os << " = " << *initializationExpression;
  }
  os << std::endl;
  return os;
}

std::ostream & NFunctionDeclaration::print(std::ostream & os) const
{
  os << "Function declared --- (" << type << " " << ident << "(";
  for (int i = 0; i < variables.size(); i++)
    {
      os << *(variables[i]) << " ";
    }
  os << ") " << *body << ")"; 
  os << std::endl;
  
  return os;
}

std::ostream & NStructureDeclaration::print(std::ostream & os) const
{
  os << "Struct declared --- (" << ident << " {";
  for (int i = 0; i < members.size(); i++)
    {
      os << *(members[i]) << " ";
    }
  os << "})";
  os << std::endl;
  
  return os;
}

std::ostream & NAssignmentStatement::print(std::ostream & os) const
{
  os << "(Assignment: " << ident << " = " << expr << ")" << std::endl;
  return os;
}

std::ostream & NListAssignmentStatement::print(std::ostream & os) const
{
  os << "(Assignment: " << list << " = " << expr << "<" << std::endl;
  return os;
}

std::ostream & NStructureAssignmentStatement::print(std::ostream & os) const
{
  os << "(Assignment: " << structure << " = " << expr << ")" << std::endl;
  return os;
}

std::ostream & NBinaryOperator::print(std::ostream & os) const
{
  std::string symbol;
  switch(op) {
      case 288: symbol = "*"; break;
      case 289: symbol = "+"; break;
      case 290: symbol = "/"; break;
      case 291: symbol = "-"; break;
  }
  os << "(BINOP: " << lhs << " " << symbol << " " << rhs << ")" << std::endl;
  return os;
}

std::ostream & NIdentifier::print(std::ostream & os) const
{
  os << "Identifier: " << value;
  return os;
}

std::ostream & NListAccess::print(std::ostream & os) const
{
  os << "(List access: " << ident << "[" << index << "])";
  return os;
}

std::ostream & NVariableAccess::print(std::ostream & os) const
{
  os << "(Variable access: " << ident << ")";
  return os;
}

std::ostream & NStructureAccess::print(std::ostream & os) const
{
  os << "(Struct access: " << ident << "." << member << ")";
  return os;
}

std::ostream & NReturn::print(std::ostream & os) const
{
  os << "(Return: " << retExpr << ")";
  os << std::endl;
  return os;
}

std::ostream & NDouble::print(std::ostream & os) const
{
  os << "DOUBLE:" << value;
  return os;
}

std::ostream & NBool::print(std::ostream & os) const
{
  os << "BOOL:" << value;
  return os;
}

std::ostream & NInt::print(std::ostream & os) const
{
  os << "INT:" << value;
  return os;
}

std::ostream & NValue::print(std::ostream & os) const
{
  os << "(Generic NValue)" << std::endl;
  return os;
}

std::ostream & NUInt::print(std::ostream & os) const
{
  os << "UINT:" << value;
  return os;
}

std::ostream & NString::print(std::ostream & os) const
{
  os << "STRING:" << value;
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
    os << "List: [";
    for (int i = 0; i < value.size(); i++)
    {
	os << *(value[i]) << " ";
    }
    os << "]" << std::endl;
    return os;
}
