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
#include "parser.h"
#include "types.h"

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
    //llvm::FunctionType *ftype = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), argTypes, false);
    llvm::FunctionType *ftype = llvm::FunctionType::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), argTypes, false);
    mainFunction = llvm::Function::Create(ftype, llvm::GlobalValue::InternalLinkage, "main", rootModule);
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", mainFunction, 0);

    variables.push_back(*(new std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> >()));
    blocks.push(bb);
    // Call codeGen on our rootBlock
    rootBlock->codeGen(*this);
    
    llvm::ReturnInst::Create(llvm::getGlobalContext(), bb);
    blocks.pop();
}

static inline llvm::Value * binOpInstCreate(llvm::Instruction::BinaryOps i, CodeGenContext & context, NExpression & lhs, NExpression & rhs)
{
    return llvm::BinaryOperator::Create(i, lhs.codeGen(context), rhs.codeGen(context), "", context.blocks.top());
}

static inline llvm::Value * cmpOpInstCreate(llvm::Instruction::OtherOps i, unsigned short p, CodeGenContext & context, NExpression & lhs, NExpression & rhs)
{
    return llvm::CmpInst::Create(i, p, lhs.codeGen(context), rhs.codeGen(context), "", context.blocks.top());
}

/**
   Function to execute a program after it's been generated using the LLVM JIT
*/
llvm::GenericValue CodeGenContext::runProgram()
{
    LLVMInitializeNativeTarget();
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
    std::vector<std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> > >::reverse_iterator scopes = variables.rbegin();
    for ( ; scopes != variables.rend(); scopes++)
    {
	if ((*scopes).find(ident) != (*scopes).end())
	{
	    return (*scopes).find(ident)->second.second;
	}
    }
    std::cout << "Unable to find variable " << ident << "!" << std::endl;
    return NULL;
}

/**
   Adds a variable name/Value * pair to the stack of scopes

   @param ident String name of variable
   @param value Pointer to llvm::Value to store
*/
void CodeGenContext::addVariable(NVariableDeclaration * var, llvm::Value * value)
{
    variables.back()[var->ident.value] = *(new std::pair<NVariableDeclaration *, llvm::Value *>(var, value));
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

llvm::Value * NIfStatement::codeGen(CodeGenContext & context)
{
    llvm::Value * cond = condition.codeGen(context);
    switch (condition.type.typecode)
    {
    case DOUBLE:
	cond = llvm::CmpInst::Create(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_ONE, llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(0.0)), cond, "", context.blocks.top());
	break;
    case UINT:
    case INT:
	cond = llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64, 0, false)), cond, "", context.blocks.top());
	break;
    default:
	cond = NULL;
	std::cout << "Error, unable to emit conditional bytecode for type: " << condition.type << std::endl;
	return cond;
	break;
    }

    llvm::Function * parent = context.blocks.top()->getParent();
    llvm::BasicBlock * tb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then", parent);
    llvm::BasicBlock * eb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else");
    llvm::BasicBlock * ifcb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "ifcont");

    llvm::BranchInst::Create(tb, eb, cond, context.blocks.top());
    
    context.Builder->SetInsertPoint(tb);
    context.blocks.push(tb);
    context.variables.push_back(*(new std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> >()));
    
    llvm::Value * tv = thenblock.codeGen(context);

    context.Builder->CreateBr(ifcb);
    tb = context.Builder->GetInsertBlock();
    context.blocks.pop();
    context.variables.pop_back();

    parent->getBasicBlockList().push_back(eb);
    context.Builder->SetInsertPoint(eb);

    context.blocks.push(eb);
    context.variables.push_back(*(new std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> >()));

    llvm::Value * ev = NULL;
    if (elseblock)
    {
	ev = elseblock->codeGen(context);
    }
    else if(elseif)
    {
	ev = elseif->codeGen(context);
    }
    context.Builder->CreateBr(ifcb);
    eb = context.Builder->GetInsertBlock();
    context.blocks.pop();
    context.variables.pop_back();

    parent->getBasicBlockList().push_back(ifcb);
    context.Builder->SetInsertPoint(ifcb);
    llvm::PHINode *PN = context.Builder->CreatePHI(llvm::Type::getVoidTy(llvm::getGlobalContext()), 2, "iftmp");

    PN->addIncoming(tv, tb);
    PN->addIncoming(ev, eb);

    return PN;
}

llvm::Value * NVariableAccess::codeGen(CodeGenContext & context)
{
    llvm::Value * var = context.findVariable(ident.value);
    llvm::Value * li = new llvm::LoadInst(var, "", false, context.blocks.top());
    return li;
}

llvm::Value * NAssignmentStatement::codeGen(CodeGenContext & context)
{
    return new llvm::StoreInst(expr.codeGen(context), context.findVariable(ident.value), false, context.blocks.top());
}

llvm::Value * NBinaryOperator::codeGen(CodeGenContext & context)
{
    switch (op)
    {
	// Math operations
    case TADD:
	return binOpInstCreate(llvm::Instruction::Add, context, lhs, rhs);
	break;
    case TSUB:
	return binOpInstCreate(llvm::Instruction::Sub, context, lhs, rhs);
	break; 
    case TMUL:
	return binOpInstCreate(llvm::Instruction::Mul, context, lhs, rhs);
	break; 
    case TDIV:
	return binOpInstCreate(llvm::Instruction::SDiv, context, lhs, rhs);
	break; 
    case TMOD:
	return binOpInstCreate(llvm::Instruction::SRem, context, lhs, rhs);
	break;

	// Comparison operations
    case TCEQ:
	if (type.typecode == DOUBLE)
	{
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_OEQ, context, lhs, rhs);
	}
	if (type.typecode == INT)
	{
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_EQ, context, lhs, rhs);
	}
	return NULL;
	break;
    case TCNEQ:
	if (type.typecode == DOUBLE)
	{
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_ONE, context, lhs, rhs);
	}
	if (type.typecode == INT)
	{
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, context, lhs, rhs);
	}
	return NULL;
	break;
    case TCLT:
	if (type.typecode == DOUBLE)
	{
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_OLT, context, lhs, rhs);
	}
	if (type.typecode == INT)
	{
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SLT, context, lhs, rhs);
	}
	return NULL;
	break;
    case TCGT:
	if (type.typecode == DOUBLE)
	{
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_OGT, context, lhs, rhs);
	}
	if (type.typecode == INT)
	{
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SGT, context, lhs, rhs);
	}
	return NULL;
	break;
    case TCLE:
	if (type.typecode == DOUBLE)
	{
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_OLE, context, lhs, rhs);
	}
	if (type.typecode == INT)
	{
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SLE, context, lhs, rhs);
	}
	return NULL;
	break;
    case TCGE:
	if (type.typecode == DOUBLE)
	{
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_OGE, context, lhs, rhs);
	}
	if (type.typecode == INT)
	{
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SGE, context, lhs, rhs);
	}
	return NULL;
	break;
default:
	return NULL;
    }
}

llvm::Value * NFunctionDeclaration::codeGen(CodeGenContext & context)
{
    std::vector<llvm::Type *> v;
    // Loop through argument types
    for (int i = 0; i < variables.size(); i++)
    {
	v.push_back(variables[i]->type.toLlvmType());
    }
    // Convert from std::vector to llvm::ArrayRef
    llvm::ArrayRef<llvm::Type *> argtypes(v);
    llvm::FunctionType *ft = llvm::FunctionType::get(type.toLlvmType(), argtypes, false);

    llvm::Function *func = llvm::Function::Create(ft, llvm::GlobalValue::InternalLinkage, ident.value.c_str(), context.rootModule);
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", func, 0);
    
    context.blocks.push(bb);
    context.variables.push_back(*(new std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> >()));

    int i = 0;
    for (llvm::Function::arg_iterator args = func->arg_begin(); args != func->arg_end(); ++args)
    {
	new llvm::StoreInst(args, variables[i]->codeGen(context), false, context.blocks.top());	
	i++;
    }

    body->codeGen(context);

    // Add duplicate return in the event there isn't one defined
    llvm::ReturnInst::Create(llvm::getGlobalContext(), bb);

    context.blocks.pop();
    context.variables.pop_back();
    
    return func;
}

llvm::Value * NFunctionCall::codeGen(CodeGenContext & context)
{
    llvm::Function *func = context.rootModule->getFunction(ident.value.c_str());
    std::vector<llvm::Value *> v;
    for (int i = 0; i < args.size(); i++)
    {
	v.push_back(args[i]->codeGen(context));
    }
    llvm::ArrayRef<llvm::Value *> llvmargs(v);
    return llvm::CallInst::Create(func, llvmargs, "", context.blocks.top());
}

llvm::Value * NReturn::codeGen(CodeGenContext & context)
{
    llvm::Value *re = retExpr.codeGen(context);
    return llvm::ReturnInst::Create(llvm::getGlobalContext(), re, context.blocks.top());
}

llvm::Value * NVariableDeclaration::codeGen(CodeGenContext & context)
{
    llvm::AllocaInst *a = new llvm::AllocaInst(type.toLlvmType(), ident.value, context.blocks.top());
    context.addVariable(this, a);

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
