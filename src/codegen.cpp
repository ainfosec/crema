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

CodeGenContext rootCodeGenCtx; 

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

llvm::Value * NBlock::codeGen(CodeGenContext & context)
{
    llvm::Value * last;
    for (int i = 0; i < statements.size(); i++)
    {
	last = statements[i]->codeGen(context);
    }
    
    return last;
}

llvm::Value * NDouble::codeGen(CodeGenContext & context)
{
    return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(value));
}

llvm::Value * NUInt::codeGen(CodeGenContext & context)
{
    return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64, value, false));
}

llvm::Value * NInt::codeGen(CodeGenContext & context)
{
    return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64, value, true));
}
