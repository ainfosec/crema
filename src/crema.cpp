#include <iostream>
#include "ast.h"

extern NBlock *rootBlock;
extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();
    if (rootBlock)
      {
	std::cout << *rootBlock << std::endl;
	if (rootBlock->semanticAnalysis(&rootCtx))
	  {
	    std::cout << "Passed semantic analysis!" << std::endl;
	  }
	else
	  {
	    std::cout << "Failed semantic analysis!" << std::endl;
	    return -1;
	  }
      }
    return 0;
}
