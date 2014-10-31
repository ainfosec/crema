/**
   @file codegen.cpp
   @brief Contains routines for generating LLVM bytecode from an AST
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   This file contains all the routines and logic to take a properly
   analyzed AST and generate LLVM IR bytecode
*/

#include "codegen.h"
#include "ast.h"

llvm::Module *rootModule;
static llvm::IRBuilder<> Builder(llvm::getGlobalContext());

/**
   An error function for code generation routines

   @param str A string to print as an error message
   @return NULL llvm:Value pointer
*/
llvm::Value * CodeGenError(std::string & str)
{
    std::cout << "ERROR: " << str << std::endl;
    return NULL;
}

llvm::Value * NDouble::codeGen(CodeGenContext & context)
{
    return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(value));
}
