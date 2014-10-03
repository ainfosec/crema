#include "ast.h"

std::ostream & operator<<(std::ostream & os, const NBlock & block)
{
    os << "Block:" << std::endl;
    for (int i = 0; i < block.statements.size(); i++)
    {
	os << block.statements[i] << std::endl;
    }
    return os;
}

std::ostream & operator<<(std::ostream & os, const NAssignmentStatement & assignment)
{
  os << "Assignment: " << assignment.ident << " = " << assignment.expr << std::endl;
  return os;
}

std::ostream & operator<<(std::ostream & os, const NIdentifier & ident)
{
  os << "Identifier: " << ident.value;
  return os;
}

std::ostream & operator<<(std::ostream & os, const NString & stringValue)
{
  os << "String: " << stringValue.value;
  return os;
}

std::ostream & operator<<(std::ostream & os, const NLoopStatement & loop)
{
  os << "Loop: " << loop.list << " as " << loop.asVar << std::endl;
  os << "{" << loop.loopBlock << "}" << std::endl;
  return os;
}

std::ostream & operator<<(std::ostream & os, const NFunctionCall & funcCall)
{
  os << "Function Call: " << funcCall.ident << std::endl;
    for (int i = 0; i < funcCall.args.size(); i++)
    {
	os << funcCall.args[i] << std::endl;
    }
    return os;
}
