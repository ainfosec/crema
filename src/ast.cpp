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
