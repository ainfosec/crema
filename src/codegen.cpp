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
std::map<std::string, std::pair<NStructureDeclaration *, llvm::StructType *> > structs;

/**
   This constructor creates an llvm::Module object called 'rootModule' and a llvm::IRBuilder
   object called 'Builder'. A Module instance is used to store all the information related to
   an LLVM module. Modules are the top-level container of all other LLVM IR objects. Each
   module contains a list of global variables, functions, and libraries that it depends on, 
   a symbol table, and data about the target's characteristics. The IRBuilder provides a uniform
   API for creating instructions and inserting them into a basic block. Use mutators 
   (e.g. setVolatile) on instructions after they have been created for access to extra instruction
   properties.
*/
CodeGenContext::CodeGenContext()
{
    rootModule = new llvm::Module("Crema JIT", llvm::getGlobalContext());
    rootModule->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    Builder = new llvm::IRBuilder<>(llvm::getGlobalContext());
}

/**
   Function to generate the LLVM IR bytecode for the program. 
   llvm::ArrayRef<T> -- constant reference to an array and allows various APIs to take consecutive
    elements easily and conveniently.
   llvm::Type -- instances are immutable and only one instance of a particular type is ever created.
    Thus, seeing if two types are equal is a pointer comparison. Once allocated, Types are never 
    freed.
   llvm::FunctionType -- derived from llvm::Type
   llvm::Function -- derived from llvm::GlobalObject, immutable at runtime, because addess is 
    immutable.
   llvm::BasicBlock -- a container of instructions that execute sequentially. A well-formed basic
    block has a list of non-terminating instructions followed by a single TerminatorInt instruction.
   variables is of type vector<map, pair, llvm::Value *>
   blocks is of type stack<llvm::BasicBlock *>
   llvm::ReturnInst -- return a value (possibly void) from a function, and execution does not 
    continue

   @param rootBlock Pointer to the root NBlock for the program
*/
void CodeGenContext::codeGen(NBlock * rootBlock)
{
    Type * ct = new Type();
    ct->typecode = CHAR;
    ct->isList = false;

    // Define arguments to root function, i.e. main(int, char**)
    std::vector<llvm::Type *> params;
    params.push_back(llvm::Type::getInt64Ty(llvm::getGlobalContext()));
    params.push_back(llvm::PointerType::get(llvm::Type::getInt8PtrTy(llvm::getGlobalContext()), 0));
    llvm::ArrayRef<llvm::Type *> argTypes(params);
    
    // Create the root "function" for top level functionality
    llvm::FunctionType *ftype = llvm::FunctionType::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), argTypes, false);
    mainFunction = llvm::Function::Create(ftype, llvm::GlobalValue::ExternalLinkage, "main", rootModule);
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", mainFunction, 0);

    variables.push_back(*(new std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> >()));
    blocks.push(bb);
    if (rootBlock)
      {		
	llvm::Function::arg_iterator args = mainFunction->arg_begin(); 	
	std::vector<llvm::Value *> argvParseV;
	args->setName("argc");
	argvParseV.push_back(args);
	args++;
	args->setName("argv");
	argvParseV.push_back(args);
	llvm::ArrayRef<llvm::Value *> argvParseR(argvParseV);

	// Create stdlib function (defined in stdlib/stdlib.c)
	// stdlib functions defined in ast.cpp are not yet available

	std::string funcname = "save_args";
	llvm::FunctionType *ft = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), argTypes, false);
	llvm::Function * func = llvm::Function::Create(ft, llvm::GlobalValue::ExternalLinkage, funcname.c_str(), this->rootModule);
	llvm::CallInst::Create(func, argvParseR, "", this->blocks.top());

	// Call codeGen on our rootBlock
	rootBlock->codeGen(*this);
      }
    llvm::ReturnInst::Create(llvm::getGlobalContext(), llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64, 0, true)), blocks.top());
    blocks.pop();
}

/** 
   This function is a quick and dirty way to execute an 'sitofp' instruction to double
*/
static inline llvm::CastInst* convertIToFP(llvm::Value * toConvert, CodeGenContext & context)
{
    return new llvm::SIToFPInst(toConvert, llvm::Type::getDoubleTy(llvm::getGlobalContext()), "", context.blocks.top());
}

/**
   A generic casting instruction generator 
   
   @param val llvm::Value to cast
   @param expr Pointer to NExpression that generated the passed val
   @param type Type to convert to
   @param context CodeGenContext
   
   @return Generated llvm::Value * for casting instruction or error
*/
llvm::Value * convertToType(llvm::Value * val, NExpression * expr, Type & type, CodeGenContext & context)
{
  if (expr->type == type)
    {
      return val;
    }
  if (expr->type.typecode == INT && type.typecode == DOUBLE)
    {
      return convertIToFP(val, context);
    }
  //if (expr.type.typecode == UINT && type.typecode == DOUBLE)
  return CodeGenError("Unable to generate casting instruction!");
}

/**
   This function returns an instance of the llvm::BinaryOperator object generated with the Create function. 
   This is a construction of a binary instruction given the opcode and the two operands. 

   @param llvm::Instruction::BinaryOps i -- enum containing #defines and #includes
   @param CodeGenContext & context -- reference to the context of the operator statement  
   @param NExpression & lhs -- lhs operand
   @param NExpression & rhs -- rhs operand
   @return llvm::Value * -- Pointer to an llvm::BinaryOperator instance containing instructions.
*/
static inline llvm::Value * binOpInstCreate(llvm::Instruction::BinaryOps i, CodeGenContext & context, NExpression & lhs, NExpression & rhs)
{
  if (rhs.type == lhs.type)
    {
      return llvm::BinaryOperator::Create(i, lhs.codeGen(context), rhs.codeGen(context), "", context.blocks.top());
    }
  Type & lt = Type::getLargerType(lhs.type, rhs.type);
  return llvm::BinaryOperator::Create(i, convertToType(lhs.codeGen(context), &lhs, lt, context), convertToType(rhs.codeGen(context), &rhs, lt, context), "", context.blocks.top());
}

/**
   Creates an llvm::CmpInst::Create object, which constructs a compare instruction, given the predicate and two operands. The instruction is then 
   inserted into a BasicBlock before the specified instruction. 

   @param llvm::Instruction::OtherOps i -- an enum data structure
   @param unsigned short p -- an enum that lists the possible predicates for CmpInst subclasses
   @param CodeGenContext & context -- reference to the context of the operator statement
   @param NExpression & lhs -- lhs operand
   @param NExpression & rhs -- rhs operand
   @return llvm::Value * -- Pointer to an llvm::CmpInst instance containing instructions.
*/
static inline llvm::Value * cmpOpInstCreate(llvm::Instruction::OtherOps i, unsigned short p, CodeGenContext & context, NExpression & lhs, NExpression & rhs)
{
  // ********* NOTE **********
  // The OtherOps needs to be
  // mapped to the appropriate
  // tokens.
  // *************************
  if (rhs.type == lhs.type)
    {
      return llvm::CmpInst::Create(i, p, lhs.codeGen(context), rhs.codeGen(context), "", context.blocks.top());
    }
  Type & lt = Type::getLargerType(lhs.type, rhs.type);
  return llvm::CmpInst::Create(i, p, convertToType(lhs.codeGen(context), &lhs, lt, context), convertToType(rhs.codeGen(context), &rhs, lt, context), "", context.blocks.top());
}


/**
   Function to execute a program after it's been generated using the LLVM JIT
   LLVMInitializeNativeTarget() -- initializes the native target corresponding to the host, useful to ensure target is linked correctly
   llvm::ExecutionEngine -- abstract interface for implementation execution of LLVM modules

   @return llvm::GenericValue object -- struct data structure
*/
llvm::GenericValue CodeGenContext::runProgram()
{
    std::string err;
    LLVMInitializeNativeTarget();
    llvm::ExecutionEngine *ee = llvm::EngineBuilder(rootModule).setEngineKind(llvm::EngineKind::Interpreter).setErrorStr(&err).create();
    if (!ee)
    {
	std::cout << "Error: " << err << std::endl;
	exit(-1);
    }
    
    llvm::ArrayRef<llvm::GenericValue> noargs;
    llvm::GenericValue retVal = ee->runFunction(mainFunction, noargs);
    
    return retVal;
}

/**
   Looks up a reference to a variable by iterating over the variables vector and searching for the string
   name of the variable (ident). 

   @param ident String name of variable
   @return Pointer to llvm::Value of that variable, NULL if variable not found.
*/
llvm::Value * CodeGenContext::findVariable(std::string ident)
{
    std::vector<std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> > >::reverse_iterator scopes;
    for ( scopes = variables.rbegin(); scopes != variables.rend(); scopes++)
       if ((*scopes).find(ident) != (*scopes).end())
	      return (*scopes).find(ident)->second.second;
    
    std::cout << "Unable to find variable " << ident << "!" << std::endl;
    return NULL;
}

/**
   Looks up a reference to a variable by iterating over the variables vector and searching for the string
   name of the variable (ident). 

   @param ident String name of variable
   @return Pointer to NVariableDeclaration of that variable, NULL if variable not found.
*/
NVariableDeclaration * CodeGenContext::findVariableDeclaration(std::string ident)
{
    std::vector<std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> > >::reverse_iterator scopes;
    for ( scopes = variables.rbegin(); scopes != variables.rend(); scopes++)
       if ((*scopes).find(ident) != (*scopes).end())
	      return (*scopes).find(ident)->second.first;
    
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
llvm::Value * CodeGenError(const char * str)
{
    std::cout << "ERROR: " << str << std::endl;
    return NULL;
}

/**
   Iterates over the StatementList statements vector (typedef std::vector<NStatement*> StatementList) and
   returns a pointer to the last statement in the context.

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the last statement in the codeGen(context)
*/
llvm::Value * NBlock::codeGen(CodeGenContext & context)
{
    llvm::Value * last;
    for (auto it : statements)
        last = (it)->codeGen(context);

    return last;
}

/**
   Generates code to declare a structure. This will take the format of the structure and create a new
   LLVM StructType which is later used in the codeGen method for NVariableDeclaration.

   @param context CodeGenContext parameter
   @return Returns nothing concretely, but adds the structure type to the struct map
*/
llvm::Value * NStructureDeclaration::codeGen(CodeGenContext & context)
{
  std::vector<llvm::Type *> vec;
  for (int i = 0; i < members.size(); i++)
    {
      vec.push_back(members[i]->type.toLlvmType());
    }
  llvm::ArrayRef<llvm::Type *> mems(vec);
  structs[ident.value] = *(new std::pair<NStructureDeclaration *, llvm::StructType *>(this, llvm::StructType::create(llvm::getGlobalContext(), mems, ident.value, false)));
}

/**
   Generates code for looping constructs

   @param context Reference of the CodeGenContext
   @return llvm::Value * pointing to the generated instructions
*/
llvm::Value * NLoopStatement::codeGen(CodeGenContext & context)
{
    NIdentifier * itIdent = new NIdentifier("loopItCnter");
    NVariableDeclaration * loop = context.findVariableDeclaration(list.value);
    NVariableDeclaration * loopVar = new NVariableDeclaration(*(new Type(loop->type, false)), asVar, NULL);
    NVariableDeclaration * itNum = new NVariableDeclaration(*(new Type(TTINT)), *itIdent, new NInt(0));
    
    std::vector<NExpression *> args;
    args.push_back(new NVariableAccess(list));
    NVariableAccess * access = new NVariableAccess(*itIdent);
    access->type = itNum->type;
    NFunctionCall * funcCall = new NFunctionCall(*(new NIdentifier("list_length")), args);
    funcCall->type = itNum->type;
    NBinaryOperator * c = new NBinaryOperator(*((NExpression *) access), (int) TCEQ, *((NExpression *) funcCall));
    llvm::Value * cond;
    llvm::Function * parent = context.blocks.top()->getParent();
    llvm::BasicBlock * preBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "preblock", parent);
    llvm::BasicBlock * bodyBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "bodyblock", parent);
    llvm::BasicBlock * terminateBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "termblock");

    // Create pre-block
    context.blocks.push(preBlock);
    llvm::Value * itNumBC = itNum->codeGen(context);
    llvm::Value * lvBC = loopVar->codeGen(context);
    llvm::BranchInst::Create(bodyBlock, context.blocks.top()->end());
    context.blocks.pop();
    
    std::cout << "Creating branch to preBlock" << std::endl;
    llvm::BranchInst::Create(preBlock, context.blocks.top()->end());

    context.blocks.push(bodyBlock);
    context.variables.push_back(*(new std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> >()));
    // Add asVar to context
    context.addVariable(loopVar, lvBC);
    
    NListAccess * nla = new NListAccess(list, access);
    nla->type = loop->type;
    new llvm::StoreInst(nla->codeGen(context), lvBC, false, context.blocks.top());
    
    std::cout << "Generating body" << std::endl;
    llvm::Value * bodyval = loopBlock.codeGen(context);

    // Increment loop counter
    NInt one(1);

    llvm::Value * newValue = llvm::BinaryOperator::Create(llvm::Instruction::Add, one.codeGen(context), new llvm::LoadInst(itNumBC, "", false, context.blocks.top()), "", context.blocks.top());
    new llvm::StoreInst(newValue, itNumBC, false, context.blocks.top());

    // Create termination check
    cond = c->codeGen(context);
    llvm::BranchInst::Create(terminateBlock, bodyBlock, cond, context.blocks.top());
    
    context.blocks.pop();
    context.variables.pop_back();

    // Link in the terminate block to the function
    context.blocks.push(terminateBlock);
    parent->getBasicBlockList().push_back(terminateBlock);
    
    return cond;
}

/**
   Generates the code for if/if-then/if-then-else statements. A switch statement assigns the value to an llvm::Value * cond variable.
   Then pointers to the parent block and the then, else, and ifcond blocks are created. The code then goes through and builds the
   set of instructions according to the construction of the if statement. 

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the llvm::Value object containing the generated code
*/
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
    case BOOL:
      break;
    default:
	    cond = NULL;
	    std::cout << "Error, unable to emit conditional bytecode for type: " << condition.type << std::endl;
	    return cond;
	    break;
    }

    llvm::Function * parent = context.blocks.top()->getParent();
    llvm::BasicBlock * thenBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then", parent);
    llvm::BasicBlock * elseBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else", parent);
    llvm::BasicBlock * ifcontBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "ifcont");

    llvm::BranchInst::Create(thenBlock, elseBlock, cond, context.blocks.top());
    
    context.Builder->SetInsertPoint(thenBlock);
    context.blocks.push(thenBlock);
    context.variables.push_back(*(new std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> >()));
    
    llvm::Value * thenValue = thenblock.codeGen(context);

    context.Builder->CreateBr(ifcontBlock);
    thenBlock = context.Builder->GetInsertBlock();
    context.blocks.pop();
    context.variables.pop_back();

    parent->getBasicBlockList().push_back(elseBlock);
    context.Builder->SetInsertPoint(elseBlock);

    context.blocks.push(elseBlock);
    context.variables.push_back(*(new std::map<std::string, std::pair<NVariableDeclaration *, llvm::Value *> >()));

    llvm::Value * elseValue = NULL;
    
    if (elseblock)
       elseValue = elseblock->codeGen(context);
    else if(elseif)
       elseValue  = elseif->codeGen(context);
    
    context.Builder->CreateBr(ifcontBlock);
    elseBlock = context.Builder->GetInsertBlock();
    context.blocks.pop();
    context.variables.pop_back();

    context.blocks.push(ifcontBlock);
    parent->getBasicBlockList().push_back(ifcontBlock);
    context.Builder->SetInsertPoint(ifcontBlock);
/*
    llvm::PHINode *PN = context.Builder->CreatePHI(llvm::Type::getVoidTy(llvm::getGlobalContext()), 2, "iftmp");

    PN->addIncoming(thenValue, thenBlock);
    PN->addIncoming(ev, elseBlock);

    return PN;
*/
    return cond;
}

/**
   Generates the code to read a variable from memory. The 'false' flag indicates that the variable is NOT volatile.
   
   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code of the loaded instruction that was generated.
*/
llvm::Value * NVariableAccess::codeGen(CodeGenContext & context)
{
    llvm::Value * var = context.findVariable(ident.value);
    llvm::Value * loadedInst = new llvm::LoadInst(var, "", false, context.blocks.top());
    return loadedInst;
}

/**
   Generates the instruction code for storing to memory. The 'false' flag indicates that the variable is NOT volatile.
   
   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code of the instruction that was generated.
*/
llvm::Value * NAssignmentStatement::codeGen(CodeGenContext & context)
{
  return new llvm::StoreInst(expr.codeGen(context), context.findVariable(ident.value), false, context.blocks.top());
}

/**
   Generates the code for binary and comparison statements.

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code of the binary or comparison expression that was generated.
*/
llvm::Value * NBinaryOperator::codeGen(CodeGenContext & context)
{
    TypeCodes tc = Type::getLargerType(rhs.type, lhs.type).typecode;

    switch (op)
    {
	// Math operations
    case TADD:
      if (tc == DOUBLE)
        return binOpInstCreate(llvm::Instruction::FAdd, context, lhs, rhs);
      if (tc == INT)
    	return binOpInstCreate(llvm::Instruction::Add, context, lhs, rhs);
      break;
    case TSUB:
      if (tc == DOUBLE)
        return binOpInstCreate(llvm::Instruction::FSub, context, lhs, rhs);
      if (tc == INT)
    	return binOpInstCreate(llvm::Instruction::Sub, context, lhs, rhs);
      break; 
    case TMUL:
      if (tc == DOUBLE)
        return binOpInstCreate(llvm::Instruction::FMul, context, lhs, rhs);
      if (tc == INT)
    	return binOpInstCreate(llvm::Instruction::Mul, context, lhs, rhs);
      break; 
    case TDIV:
      if (tc == DOUBLE)
        return binOpInstCreate(llvm::Instruction::FDiv, context, lhs, rhs);
      if (tc == INT)
    	return binOpInstCreate(llvm::Instruction::SDiv, context, lhs, rhs);
      break; 
    case TMOD:
      if (tc == DOUBLE)
        return binOpInstCreate(llvm::Instruction::FRem, context, lhs, rhs);
      if (tc == INT)
    	return binOpInstCreate(llvm::Instruction::SRem, context, lhs, rhs);
      break;
    case TBAND:
       return binOpInstCreate(llvm::Instruction::And, context, lhs, rhs);
       break;
    case TBOR:
       return binOpInstCreate(llvm::Instruction::Or, context, lhs, rhs);
       break;
    case TBXOR:
        return binOpInstCreate(llvm::Instruction::Xor, context, lhs, rhs);
        break;

	// Comparison operations
    case TCEQ:
      if (tc == DOUBLE)
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_OEQ, context, lhs, rhs);
      if (tc == INT && rhs.type == lhs.type)
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_EQ, context, lhs, rhs);
      return NULL;
    break;
    
    case TCNEQ:
	  if (tc == DOUBLE)
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_ONE, context, lhs, rhs);
	  if (tc == INT)
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, context, lhs, rhs);
	  return NULL;
	  break;
    
    case TCLT:
	  if (tc == DOUBLE)
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_OLT, context, lhs, rhs);
	  if (tc == INT)
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SLT, context, lhs, rhs);
	  return NULL;
	  break;
    
    case TCGT:
	  if (tc == DOUBLE)
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_OGT, context, lhs, rhs);
	  if (tc == INT)
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SGT, context, lhs, rhs);
	  return NULL;
	  break;
    
    case TCLE:
	  if (tc == DOUBLE)
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_OLE, context, lhs, rhs);
	  if (tc == INT)
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SLE, context, lhs, rhs);
	  return NULL;
	  break;
    
    case TCGE:
	  if (tc == DOUBLE)
	    return cmpOpInstCreate(llvm::Instruction::FCmp, llvm::CmpInst::FCMP_OGE, context, lhs, rhs);
	  if (tc == INT)
	    return cmpOpInstCreate(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SGE, context, lhs, rhs);
	  return NULL;
	  break;
    
    default:
	  return NULL;
    }
}

/**
   Generates the stdlib function call bytecode to retrieve an element from a list

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated that will access a list elemnt.
*/
llvm::Value * NListAccess::codeGen(CodeGenContext & context)
{
    llvm::Value * var = context.findVariable(ident.value);
    std::string name;
    if (!index)
      {
	std::cout << "NULL index for NListAccess!" << std::endl;
	return NULL;
      }
    switch (type.typecode)
      {
      case INT:
	name = "int_list_retrieve";
	break;
      case STRING:
	  name = "string_retrieve";
	  break;
      default:
	return NULL;
      }
    llvm::Function *func = context.rootModule->getFunction(name.c_str());
    llvm::Value * li = new llvm::LoadInst(var, "", false, context.blocks.top());
    std::vector<llvm::Value *> v;
    v.push_back(li);
    v.push_back(index->codeGen(context));
    
    llvm::ArrayRef<llvm::Value *> llvmargs(v);
    return llvm::CallInst::Create(func, llvmargs, "", context.blocks.top());
}

/**
   Generates the stdlib function call bytecode to insert an element into a list

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated that will insert into a list elemnt.
*/
llvm::Value * NListAssignmentStatement::codeGen(CodeGenContext & context)
{
    llvm::Value * var = context.findVariable(list.ident.value);
    std::string name;
    switch (list.type.typecode)
      {
      case INT:
	  if (list.index)
	  {
	      name = "int_list_insert";
	  }
	  else
	  {
	      name = "int_list_append";
	  }
	  break;
      case STRING:
      case CHAR:
	  if (list.index)
	  {
	      name = "str_insert";
	  }
	  else
	  {
	      name = "str_append";
	  }
      default:
	  std::cout << "Unable to assign list for type: " << list.type.typecode << std::endl;
	  return NULL;
      }
    llvm::Function *func = context.rootModule->getFunction(name.c_str());
    llvm::Value * li = new llvm::LoadInst(var, "", false, context.blocks.top());
    std::vector<llvm::Value *> v;
    v.push_back(li);
    if (list.index)
      {
	v.push_back(list.index->codeGen(context));
      }
    v.push_back(expr.codeGen(context));

    llvm::ArrayRef<llvm::Value *> llvmargs(v);
    return llvm::CallInst::Create(func, llvmargs, "", context.blocks.top());
}

static llvm::GetElementPtrInst * getGEPForStruct(llvm::Value * var, NIdentifier & member, NStructureDeclaration * sd, CodeGenContext & context)
{
    int i;
    for (i = 0; i < sd->members.size(); i++)
    {
	if (member == sd->members[i]->ident)
	{
	    break;
	}
    }
    
    std::vector<llvm::Value *> vec;
    // The argument IdxList *MUST* contain i32 values otherwise the call to GEP::Create() will segfault
    vec.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 0, true)));
    vec.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, i, true)));
    
    llvm::ArrayRef<llvm::Value *> arr(vec);
    llvm::GetElementPtrInst * gep = llvm::GetElementPtrInst::Create(var, arr, "", context.blocks.top());
    if (!gep)
    {
	std::cout << "Error: Unable to make GEP instruction!" << std::endl;
	exit(-1);
    }
    return gep;
}

/**
   Generates LLVM IR byte-code for accessing a structure field

   @param context Reference to CodeGenContext
   @return Pointer to generated llvm::Value or NULL if code cannot be generated
*/
llvm::Value * NStructureAccess::codeGen(CodeGenContext & context)
{
    llvm::Value * var = context.findVariable(ident.value);
    NVariableDeclaration * vd = context.findVariableDeclaration(ident.value);
    if (!vd || !var)
    {
	std::cout << "Error: Unable to find variable for " << ident << std::endl;
	exit(-1);
    }
    StructType *st = (StructType *) &(vd->type);
    NStructureDeclaration * sd = structs[st->ident.value].first;
    llvm::GetElementPtrInst * gep = getGEPForStruct(var, member, sd, context);
    return new llvm::LoadInst(gep, "", false, context.blocks.top());
}

/**
   Generates the instruction code for storing to memory. The 'false' flag indicates that the variable is NOT volatile.
   
   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code of the instruction that was generated.
*/
llvm::Value * NStructureAssignmentStatement::codeGen(CodeGenContext & context)
{
    llvm::Value * var = context.findVariable(structure.ident.value);
    NVariableDeclaration * vd = context.findVariableDeclaration(structure.ident.value);
    if (!vd || !var)
    {
	std::cout << "Error: Unable to find variable for " << structure.ident << std::endl;
	exit(-1);
    }
    StructType *st = (StructType *) &(vd->type);
    NStructureDeclaration * sd = structs[st->ident.value].first;
    llvm::GetElementPtrInst * gep = getGEPForStruct(var, structure.member, sd, context);
    return new llvm::StoreInst(expr.codeGen(context), gep, false, context.blocks.top());
}

/**
   Generates the necessary code for defining a function. 

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated that will declare a function.
*/
llvm::Value * NFunctionDeclaration::codeGen(CodeGenContext & context)
{
    std::vector<llvm::Type *> v;
    // Loop through argument types
    for (auto it : variables)
        v.push_back(it->type.toLlvmType());

    // Convert from std::vector to llvm::ArrayRef
    llvm::ArrayRef<llvm::Type *> argtypes(v);
    llvm::FunctionType *ft = llvm::FunctionType::get(type.toLlvmType(), argtypes, false);
    llvm::Function * func;

    if (body)
      {
	func = llvm::Function::Create(ft, llvm::GlobalValue::InternalLinkage, ident.value.c_str(), context.rootModule);
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

	if (type.typecode == VOID)
	  {
	    // Add in a void return instruction for void functions
	    llvm::ReturnInst::Create(llvm::getGlobalContext(), bb);
	  }

	context.blocks.pop();
	context.variables.pop_back();
      }
    else 
      {
	func = llvm::Function::Create(ft, llvm::GlobalValue::ExternalLinkage, ident.value.c_str(), context.rootModule);
      }
    return func;
}

/**
   Generates the necessary code needed to call a defined function. 
   
   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated that will call a function.
*/
llvm::Value * NFunctionCall::codeGen(CodeGenContext & context)
{
    llvm::Function *func = context.rootModule->getFunction(ident.value.c_str());
    std::vector<llvm::Value *> v;
    for (auto it : args) 
        v.push_back(it->codeGen(context));
   
    llvm::ArrayRef<llvm::Value *> llvmargs(v);
    return llvm::CallInst::Create(func, llvmargs, "", context.blocks.top());
}

/**
   Generates the necessary code needed to execute a return statement.
   
   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated that will execute a return statement.
*/
llvm::Value * NReturn::codeGen(CodeGenContext & context)
{    
    llvm::Value *re = retExpr.codeGen(context);

    // upcasts the return value to floating point if function return is 
    // declared as double, but integer is returned in body of function
    if ( retExpr.type.toLlvmType() != context.blocks.top()->getParent()->getReturnType() )
        return llvm::ReturnInst::Create(llvm::getGlobalContext(), convertIToFP(re,context), context.blocks.top());

    return llvm::ReturnInst::Create(llvm::getGlobalContext(), re, context.blocks.top());
}

/**
   Generates the code for allocating memory and declaring a variable.

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated that will execute a return statement.
*/
llvm::Value * NVariableDeclaration::codeGen(CodeGenContext & context)
{
  llvm::Value * a;
  if (type.isStruct) 
    {
      StructType *st = (StructType *) &type;
      if (context.blocks.top()->getParent()->getName().str() == "main")
	{
//	    a = new llvm::AllocaInst(structs[st->ident.value].second, ident.value, context.blocks.top());
	    a = new llvm::GlobalVariable(*(context.rootModule), structs[st->ident.value].second, false, llvm::GlobalValue::InternalLinkage, llvm::UndefValue::get(structs[st->ident.value].second), ident.value);
	}
      else 
	{
	    a = new llvm::AllocaInst(structs[st->ident.value].second, ident.value, context.blocks.top());
	}
    }
  else 
    {
      if (context.blocks.top()->getParent()->getName().str() == "main")
      {
	  a = new llvm::GlobalVariable(*(context.rootModule), type.toLlvmType(), false, llvm::GlobalValue::InternalLinkage, llvm::UndefValue::get(type.toLlvmType()), ident.value);
      }
      else 
              {
	  a = new llvm::AllocaInst(type.toLlvmType(), ident.value, context.blocks.top());
      }
      if ((type.isList || type.typecode == STRING) && !initializationExpression)
      {
	  // TODO codegen
	  std::string name;
	  switch (type.typecode)
	  {
	  case INT:
	      name = "int_list_create";
	      break;
	  case STRING:
	  case CHAR:
	      name = "str_create";
	      break;
	  default:
	      std::cout << "Unable to create list for type: " << type << std::endl;
	      return NULL;
	  }
	  llvm::Function *func = context.rootModule->getFunction(name.c_str());
	  std::vector<llvm::Value *> v;
	  
	  llvm::ArrayRef<llvm::Value *> llvmargs(v);
	  llvm::Value * si = new llvm::StoreInst(llvm::CallInst::Create(func, llvmargs, "", context.blocks.top()), a, false, context.blocks.top());
      }
    }
  context.addVariable(this, a);
  
  if (NULL != initializationExpression)
  {
      NAssignmentStatement nas(ident, *initializationExpression);
      nas.codeGen(context);
  }
  
  return a;
}

/**
   Generates code for a pre-defined list

   @param context Context variable
   @return LLVM bytecode for the list
*/
llvm::Value * NList::codeGen(CodeGenContext & context)
{
    // Create list
    std::string name;
    switch (type.typecode)
    {
    case INT:
	name = "int_list_create";
	break;
    case CHAR:
	name = "str_create";
	break;
    default:
	return NULL;
    }
    llvm::Function *func = context.rootModule->getFunction(name.c_str());
    std::vector<llvm::Value *> v;
    
    llvm::ArrayRef<llvm::Value *> llvmargs(v);
    llvm::Value * li = llvm::CallInst::Create(func, llvmargs, "", context.blocks.top());
    li->dump();
    for (int i = 0; i < value.size(); i++)
    {
	switch (type.typecode)
	{
	case INT:
	    name = "int_list_append";
	    break;
	case CHAR:
	    name = "str_append";
	    break;
	default:
	    return NULL;
	}
	llvm::Function *func = context.rootModule->getFunction(name.c_str());
	std::vector<llvm::Value *> v;
	v.push_back(li);
	v.push_back(value[i]->codeGen(context));

	llvm::ArrayRef<llvm::Value *> llvmargs(v);
	llvm::CallInst::Create(func, llvmargs, "", context.blocks.top());
    }

    return li;
}

/**
   Generates code for string values.

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated as a result of the string instance
*/
llvm::Value * NString::codeGen(CodeGenContext & context)
{
    ExpressionList * chars = new ExpressionList();
    for (int i = 0; i < value.length(); i++)
    {
	NChar *c = new NChar(value[i]);
	c->type.typecode = CHAR;
	chars->push_back(c);
    }
    NList * cl = new NList(*chars);
    cl->type.typecode = CHAR;
    llvm::Value * v = cl->codeGen(context);
    return v;
}

/**
   Generates code for floating point values.

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated as a result of the floating point instance
*/
llvm::Value * NDouble::codeGen(CodeGenContext & context)
{
    return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(value));
}

/**
   Generates code for unsigned int values.

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated as a result of the unsigned integer instance
*/
llvm::Value * NUInt::codeGen(CodeGenContext & context)
{
    return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64, value, false));
}

/**
   Generates code for signed integer values.

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated as a result of the signed integer instance
*/
llvm::Value * NInt::codeGen(CodeGenContext & context)
{
    return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64, value, true));
}

/**
   Generates code for signed char values.

   @param CodeGenContext & context -- reference to the context of the operator statement
   @return llvm::Value * -- Pointer to the code generated as a result of the signed char instance
*/
llvm::Value * NChar::codeGen(CodeGenContext & context)
{
    return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(8, value, true));
}
