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
extern "C" FILE *yyin;

void yyset_debug(int);

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
    opt.add("", 0, 1, 0, "Print parser output and root block", "-v"); // TODO!

    opt.parse(argc, argv);

    if (opt.isSet("-h"))
    {
	std::string usage;
	opt.getUsage(usage);
	std::cout << usage;
	return 0;
    }
    
    if (opt.isSet("-v")) {
	yyset_debug(1);
    } else {
	yyset_debug(0);
    }
    // Parse input
    if (opt.isSet("-f")) {
        // searches for the -f flag
        int i=0;
        while (argv[i] != std::string("-f"))
            ++i;

        // reads the file name string after the -f flag
        FILE *inFile = fopen(argv[i+1],"r");
        if (!inFile)
            std::cout << "Cannot open file, " << argv[i+1] << ".\n" << "Usage: ./cremacc -f <input file>\n";
        yyin = inFile;

        // feeds the input file into cremacc
        do {
            yyparse();
        } while (!feof(yyin));
    }
    else 
        yyparse(); // no -f flag will produce the commandline cremacc

    if (opt.isSet("-p"))
    {
	return 0;
    }

    // Perform semantic checks
    if (rootBlock)
    {
        if (opt.isSet("-v")) {
	    std::cout << *rootBlock << std::endl;
        }
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

    if (opt.isSet("-c"))
    {
        // searches for the -c flag
        int i=0;
        while (argv[i] != std::string("-c"))
            ++i;

        // writes output LLVM assembly to argument after -c flag
        FILE *outFile;
        outFile = freopen(argv[i+1],"w",stderr);
        rootCodeGenCtx.dump();
        fclose(outFile);

        std::ostringstream oss;
        std::cout << "Linking with stdlib.c using clang...\n";
        oss << "clang " << argv[i+1] << " stdlib/stdlib.c";
        std::string cmd = oss.str();
        // runs the command: clang <.ll filename> <library files>
        std::system(cmd.c_str());
        std::cout << "Executing program and printing the return value...\n";
        // executes the program ./a.out and prints the return value
        std::system("./a.out; echo $?;");
    } else {
          std::cout << "Dumping generated LLVM bytecode" << std::endl;
	  rootCodeGenCtx.dump();
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
