/**
   @file codegen.h
   @brief Header file for code generation routines
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   A header containing definitions and declarations for functionality
   related to generating LLVM IR bytecode for a Crema program
*/

#ifndef CREMA_CODEGEN_H_
#define CREMA_CODEGEN_H_

#include <llvm/IR/Value.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>

class CodeGenContext
{
public:
    llvm::Module * rootModule;
    llvm::IRBuilder<> Builder;
CodeGenContext() : rootModule(new llvm::Module("Crema JIT", llvm::getGlobalContext())), Builder(llvm::getGlobalContext()) { }
    void dump() { rootModule->dump(); }
};

llvm::Value * CodeGenError(std::string & str);


#endif // CREMA_CODEGEN_H_
