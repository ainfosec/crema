/**
   @file codegen.h
   @brief Header file for code generation routines
   @copyright 2015 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   A header containing definitions and declarations for functionality
   related to generating LLVM IR bytecode for a Crema program
*/

#ifndef CREMA_CODEGEN_H_
#define CREMA_CODEGEN_H_

#include <stack>
#include <string>
#include <map>
#include <llvm/IR/Value.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/PassManager.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Support/Host.h>

class NBlock;
class NVariableDeclaration;

class CodeGenContext
{
public:
    llvm::Module * rootModule;
    llvm::IRBuilder<> * Builder;
    llvm::Function *mainFunction;
    std::stack<llvm::BasicBlock *> blocks, listblocks;
    std::vector<std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> > > variables;
//    std::vector<std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> > > functions;
    
    CodeGenContext();
    ~CodeGenContext() { delete Builder; }
    void codeGen(NBlock * rootBlock);
    llvm::Value * findVariable(std::string ident);
    NVariableDeclaration * findVariableDeclaration(std::string ident);
    void addVariable(NVariableDeclaration * var, llvm::Value * value);
    llvm::GenericValue runProgram();
    void dump() { rootModule->dump(); }
};

llvm::Value * CodeGenError(const char *);


#endif // CREMA_CODEGEN_H_
