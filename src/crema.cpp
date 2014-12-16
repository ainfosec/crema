/**
   @file crema.cpp
   @brief Main cremacc file and main() function
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   A main function to read in an input. It will parse and perform semantic
   analysis on the input and either exit(0) if it passes both, -1 otherwise.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "ast.h"
#include "codegen.h"
#include "ezOptionParser.hpp"
#include "llvm/CodeGen/AsmPrinter.h"

extern NBlock *rootBlock;
extern int yyparse();

int main(int argc, const char *argv[])
{
    // Handling command-line options
    ez::ezOptionParser opt;
    opt.overview = "Crema Compiler for Sub-Turing Complete Programs";
    opt.syntax = "cremacc [OPTIONS]";
    opt.footer = "\n(C) 2014 Assured Information Security, Inc.\n";

    opt.add("", 0, 0, 0, "Prints this help", "-h");
    opt.add("", 0, 0, 0, "Parse only: Will halt after parsing and pretty-printing the AST for the input program", "-p");
    opt.add("", 0, 0, 0, "Semantic check only: Will halt after parsing, pretty-printing and performing semantic checks on the AST for the input program", "-s");
    opt.add("", 0, 1, 0, "Print LLVM Assembly to file.", "-c");
    opt.add("", 0, 0, 0, "Run generated code", "-r");
    opt.add("", 0, 1, 0, "Read input from file instead of stdin", "-f"); // TODO!

    opt.parse(argc, argv);

    if (opt.isSet("-h"))
    {
	std::string usage;
	opt.getUsage(usage);
	std::cout << usage;
	return 0;
    }
    
    // Parse input
    yyparse();
    if (opt.isSet("-p"))
    {
	return 0;
    }

    // Perform semantic checks
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
    if (opt.isSet("-s"))
    {
	return 0;
    }

    // Code Generation
    std::cout << "Generating LLVM IR bytecode" << std::endl;
    rootCodeGenCtx.codeGen(rootBlock);

    std::cout << "Dumping generated LLVM bytecode" << std::endl;
    rootCodeGenCtx.dump();

    if (opt.isSet("-c"))
    {
        FILE *outFile;
        outFile = freopen(argv[2],"w",stderr);
        rootCodeGenCtx.dump();
        fclose(outFile);

        std::ostringstream oss;
        oss << "clang " << argv[2] << " stdlib/stdlib.c";
        std::string cmd = oss.str();
        std::system(cmd.c_str());
        std::system("./a.out; echo $?;");
    }

    // LLVM IR JIT Execution
    if (opt.isSet("-r"))
    {
	std::cout << "Running program:" << std::endl;
	std::cout << "Return value: " << rootCodeGenCtx.runProgram().IntVal.toString(10, true) << std::endl;
	std::cout << "Program run successfully!" << std::endl;
    }
    return 0;
}
