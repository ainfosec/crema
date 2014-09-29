#include <iostream>
#include "ast.h"

extern NBlock *rootBlock;
extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();
    std::cout << rootBlock << std::endl;
    return 0;
}
