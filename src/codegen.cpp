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

CodeGenContext::CodeGenContext()
{
    rootModule = new llvm::Module("Crema JIT", llvm::getGlobalContext());
    Builder = new llvm::IRBuilder<>(llvm::getGlobalContext());
}

/**
   Function to generate the LLVM IR bytecode for the program

   @param rootBlock Pointer to the root NBlock for the program
*/
void CodeGenContext::codeGen(NBlock * rootBlock)
{
    // Create the root "function" for top level functionality
    llvm::ArrayRef<llvm::Type *> argTypes;
    llvm::FunctionType *ftype = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), argTypes, false);
    mainFunction = llvm::Function::Create(ftype, llvm::GlobalValue::InternalLinkage, "main", rootModule);
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", mainFunction, 0);

    variables.push_back(*(new std::map<std::string, llvm::Value *>()));
    blocks.push(bb);
    // Call codeGen on our rootBlock
    rootBlock->codeGen(*this);
    
    llvm::ReturnInst::Create(llvm::getGlobalContext(), bb);
    blocks.pop();
}

/**
   Function to execute a program after it's been generated using the LLVM JIT
*/
llvm::GenericValue CodeGenContext::runProgram()
{
    llvm::ExecutionEngine *ee = llvm::ExecutionEngine::create(rootModule, false);
    llvm::ArrayRef<llvm::GenericValue> noargs;
    llvm::GenericValue retVal = ee->runFunction(mainFunction, noargs);
    
    return retVal;
}

/**
   Looks up a reference to a variable

   @param ident String name of variable
   @return Pointer to llvm::Value of that variable
*/
llvm::Value * CodeGenContext::findVariable(std::string ident)
{
    std::vector<std::map<std::string, llvm::Value *> >::reverse_iterator scopes = variables.rbegin();
    for ( ; scopes != variables.rend(); scopes++)
    {
	llvm::Value * v = (*scopes).find(ident)->second;
	if (v != NULL)
	{
	    return v;
	}
    }
    return NULL;
}

/**
   Adds a variable name/Value * pair to the stack of scopes

   @param ident String name of variable
   @param value Pointer to llvm::Value to store
*/
void CodeGenContext::addVariable(std::string ident, llvm::Value * value)
{
    variables.back()[ident] = value;
}


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

llvm::Value * NVariableAccess::codeGen(CodeGenContext & context)
{
    return new llvm::LoadInst(context.findVariable(ident.value), "", false, context.blocks.top());
}

llvm::Value * NAssignmentStatement::codeGen(CodeGenContext & context)
{
    return new llvm::StoreInst(expr.codeGen(context), context.findVariable(ident.value), false, context.blocks.top());
}

llvm::Value * NReturn::codeGen(CodeGenContext & context)
{
    return llvm::ReturnInst::Create(llvm::getGlobalContext(), retExpr.codeGen(context), context.blocks.top());
}

llvm::Value * NVariableDeclaration::codeGen(CodeGenContext & context)
{
    llvm::AllocaInst *a = new llvm::AllocaInst(type.toLlvmType(), ident.value, context.blocks.top());
    context.addVariable(ident.value, a);

    if (NULL != initializationExpression)
    {
	NAssignmentStatement nas(ident, *initializationExpression);
	nas.codeGen(context);
    }
    
    return a;
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
