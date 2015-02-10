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
#include <stdio.h>
#include <unistd.h>

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
    opt.add("", 0, 1, 0, "Print LLVM Assembly to file", "-S");
    opt.add("", 0, 1, 0, "Set the output program name to ARG instead of 'a.out'", "-o");
    opt.add("", 0, 1, 0, "Read input from file instead of stdin", "-f");
    opt.add("", 0, 0, 0, "Print parser output and root block", "-v");

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

    if (opt.isSet("-S"))
    {
        // searches for the -S flag
        int i=0;
        while (argv[i] != std::string("-S"))
            ++i;

        // writes output LLVM assembly to argument after -S flag
        FILE *outFile;
        outFile = freopen(argv[i+1],"w",stderr);
        rootCodeGenCtx.dump();
        fclose(outFile);
    }

    char tmpname[12] = "crematmp.ll";
    FILE *outFile;
    outFile = freopen(tmpname, "wb", stderr);
    rootCodeGenCtx.dump();
    fclose(outFile);

    std::ostringstream oss;
    std::cout << "Linking with stdlib.c using clang..." << std::endl;
    std::string outputname = "";
    if (opt.isSet("-o"))
    {
        // searches for the -S flag
        int i=0;
        while (argv[i] != std::string("-o"))
            ++i;
	outputname = "-o " + std::string(argv[i + 1]);	
    }
    oss << "clang " << outputname << " " << tmpname << " stdlib/stdlib.c -lm";
    std::string cmd = oss.str();
    // runs the command: clang <.ll filename> <library files>
    if(std::system(cmd.c_str()))
    {
	std::cout << "ERROR: Unable to build program with CLANG!" << std::endl;
	return -1;
    }
    unlink(tmpname);
    
    return 0;
}
