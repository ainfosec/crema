/**
   @file crema.cpp
   @brief Main cremacc file and main() function
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   A main function to read in an input. It will parse and perform semantic
   analysis on the input and either exit(0) if it passes both, -1 otherwise.
 */
#include <iostream>
#include "ast.h"
#include "codegen.h"

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

    std::cout << "Generating LLVM IR bytecode" << std::endl;
    rootCodeGenCtx.codeGen(rootBlock);
    
    std::cout << "Dumping generated LLVM bytecode" << std::endl;
    rootCodeGenCtx.dump();

/*
    std::cout << "Running program:" << std::endl;
    rootCodeGenCtx.runProgram();
    std::cout << "Program run successfully!" << std::endl;
*/    
    return 0;
}
