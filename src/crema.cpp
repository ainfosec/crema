#include <iostream>
#include "ast.h"

extern NBlock *rootBlock;
extern int yyparse();
SemanticContext ctx;

int main(int argc, char **argv)
{
    yyparse();
    std::cout << *rootBlock << std::endl;
    if (rootBlock->semanticAnalysis(&ctx))
      {
	std::cout << "Passed semantic analysis!" << std::endl;
      }
    else
      {
	std::cout << "Failed semantic analysis!" << std::endl;
      }
    return 0;
}
