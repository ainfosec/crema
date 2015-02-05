/**
   @file ast.cpp
   @brief Implementation file for AST-related classes
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   Stores the implementations for the AST classes. Includes pretty-printing logic
   and some operator overloads for AST manipulation and analysis.
*/
#include "ast.h"
#include "parser.h"
#include "types.h"
#include "semantics.h"

/**
   Overload for the == operator to allow for simple comparison of two NIdentifiers.
   The NIdentifier values being compared are of type string, which represent the names of 
   variables, lists, structs, struct members, etc. 

   @param i1 First NIdentifier to compare
   @param i2 Second NIdentifier to compare
   @return true if the Identifiers resolve to the same value, false otherwise
*/
bool operator==(const NIdentifier & i1, const NIdentifier & i2)
{
  return i1.value == i2.value;
}

/**
   Overload for the << operator to allow pretty printing of AST objects and AST as a whole

   @param os Output stream to print to
   @param node Node to pretty-print
   @return Output stream passed in
*/
std::ostream & operator<<(std::ostream & os, const Node & node)
{
  node.print(os);
  return os;
}

static inline NFunctionDeclaration * generateFuncDecl(Type & type, std::string name, std::vector<NVariableDeclaration *> args)
{
    NIdentifier * ident = new NIdentifier(name);
    return new NFunctionDeclaration(type, *ident, args, NULL);
}

/**
   Function to define certain standard library declarations
 */
void NBlock::createStdlib()
{
    std::vector<NVariableDeclaration *> args;
    NFunctionDeclaration *func;
    Type * ct = new Type();
    ct->typecode = CHAR;
    ct->isList = false;
    // int_list_create()
    func = generateFuncDecl(*(new Type(TTINT, true)), "int_list_create", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);

    // double_list_create()
    func = generateFuncDecl(*(new Type(TTDOUBLE, true)), "double_list_create", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);

    // str_create()
    func = generateFuncDecl(*(new Type(*ct, true)), "str_create", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);

    // list_length(list)
    args.push_back(new NVariableDeclaration(*(new Type(TTINT, true)), *(new NIdentifier("l"))));
    statements.insert(statements.begin(), generateFuncDecl(*(new Type(TTINT)), "list_length", args));
    rootCtx.registerFunc(func); 
    
    // int_list_retrieve(list, idx)
    args.push_back(new NVariableDeclaration(*(new Type(TTINT)), *(new NIdentifier("idx"))));
    statements.insert(statements.begin(), generateFuncDecl(*(new Type(TTINT)), "int_list_retrieve", args));
    rootCtx.registerFunc(func);

    // str_retrieve(list, idx)
    statements.insert(statements.begin(), generateFuncDecl(*(new Type(TTCHAR)), "str_retrieve", args));
    rootCtx.registerFunc(func);

    // double_list_retrieve(list, idx)
    statements.insert(statements.begin(), generateFuncDecl(*(new Type(TTDOUBLE)), "double_list_retrieve", args));
    rootCtx.registerFunc(func);

    // int_list_append(list, val)
    func = generateFuncDecl(*(new Type(TTVOID)), "int_list_append", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);
    
    // int_list_insert(list, idx, val)
    args.push_back(new NVariableDeclaration(*(new Type(TTINT)), *(new NIdentifier("val"))));
    func = generateFuncDecl(*(new Type(TTVOID)), "int_list_insert", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);

    // double_list_append(l, val)
    args.clear();
    args.push_back(new NVariableDeclaration(*(new Type(TTDOUBLE, true)), *(new NIdentifier("l"))));
    args.push_back(new NVariableDeclaration(*(new Type(TTDOUBLE)), *(new NIdentifier("val"))));
    func = generateFuncDecl(*(new Type(TTVOID)), "double_list_append", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);

    // double_list_insert(l, idx, val)
    args.clear();
    args.push_back(new NVariableDeclaration(*(new Type(TTDOUBLE, true)), *(new NIdentifier("l"))));
    args.push_back(new NVariableDeclaration(*(new Type(TTINT)), *(new NIdentifier("idx"))));
    args.push_back(new NVariableDeclaration(*(new Type(TTDOUBLE)), *(new NIdentifier("val"))));
    func = generateFuncDecl(*(new Type(TTVOID)), "double_list_insert", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);    
    
    // str_print(list) & str_println(list)
    args.clear();
    args.push_back(new NVariableDeclaration(*(new Type(TTINT, true)), *(new NIdentifier("l"))));
    func = generateFuncDecl(*(new Type(TTVOID)), "str_print", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);
    func = generateFuncDecl(*(new Type(TTVOID)), "str_println", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);

    // str_append(list, val)
    args.push_back(new NVariableDeclaration(*ct, *(new NIdentifier("val"))));
    std::cout << *args[1] << std::endl;
    func = generateFuncDecl(*(new Type(TTVOID)), "str_append", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);

    // print_int(val)
    args.clear();
    args.push_back(new NVariableDeclaration(*(new Type(TTINT)), *(new NIdentifier("val"))));
    func = generateFuncDecl(*(new Type(TTVOID)), "print_int", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);
    
    // str_insert(list, idx, val)
    args.clear();
    args.push_back(new NVariableDeclaration(*(new Type(CHAR, true)), *(new NIdentifier("l"))));
    args.push_back(new NVariableDeclaration(*(new Type(TTINT)), *(new NIdentifier("idx"))));
    args.push_back(new NVariableDeclaration(*ct, *(new NIdentifier("val"))));
    func = generateFuncDecl(*(new Type(TTVOID)), "str_insert", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);  
    
    // list_t * prog_argument(int)
    args.clear();
    args.push_back(new NVariableDeclaration(*(new Type(TTINT)), *(new NIdentifier("idx"))));
    func = generateFuncDecl(*(new Type(TTCHAR, true)), "prog_argument", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);  

    // uint64_t prog_arg_count()
    args.clear();
    func = generateFuncDecl(*(new Type(TTINT)), "prog_arg_count", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);  
    
    // crema_seq(start, end)
    args.clear();
    args.push_back(new NVariableDeclaration(*(new Type(TTINT)), *(new NIdentifier("start"))));
    args.push_back(new NVariableDeclaration(*(new Type(TTINT)), *(new NIdentifier("end"))));
    func = generateFuncDecl(*(new Type(TTINT, true)), "crema_seq", args);
    statements.insert(statements.begin(), func);
    rootCtx.registerFunc(func);
}

/**
   Print function for NBlock objects in the AST. Each block is composed of a vector of 
   statements. This function iterators over the typedef std::vector<NStatement*> StatementList,
   printing each each statement in a given block, and returning the output stream.

   @param os Output stream to print to
   @return Output stream passed in
*/
std::ostream & NBlock::print(std::ostream & os) const
{
    os << "Block: {" << std::endl;
  
    for (StatementList::const_iterator it = statements.begin(); it != statements.end(); ++it)
      os << *(*it) << std::endl;

    os << "}" << std::endl;
    return os;
}

/**
   Print function for NVariableDeclaration objects. There are three members of the 
   NVariableDeclaration object that are printed: 
   
   1) Type 
   2) NIdentifier (Variable name)
   3) Associated value or RHS (optional)

   The initializationExpression is the RHS in variable assignments (e.g. the 4 in int a = 4).
   If initializationExpression is NULL, then the no value was assigned to variable (e.g. int a).

   @param os Output stream to print to
   @return Output stream passed in
*/
std::ostream & NVariableDeclaration::print(std::ostream & os) const
{
  if (!type.isList)
  {
      os << "Variable declared --- (" << type << " " << ident << ")";
  }
  else
  {
      os << "List declared --- (" << type << " " << ident << "[])";
  }
  if (initializationExpression != NULL)
  {
      os << " = " << *initializationExpression;
  }
  os << std::endl;
  return os;
}

/**
   Print function for NFunctionDeclaration objects. It first prints the return type
   and name of the function followed by the arguments contained within the 
   typedef std::vector<NVariableDeclaration *> VariableList. Then, the NBlock body 
   of the function is printed.

   @param os Output stream to print to
   @return Output stream passed in
*/
std::ostream & NFunctionDeclaration::print(std::ostream & os) const
{
  os << "Function declared --- (" << type << " " << ident << "(";

  for (VariableList::const_iterator it = variables.begin(); it != variables.end(); ++it)
      os << *(*it) << ") ";

  if (body)
    {
      os << *body << ")"; 
      os << std::endl;
    }
  return os;
}

/**
   Print function for NStructureDeclaration objects. It first prints the name of the
   struct followed by its typedef std::vector<NVariableDeclaration *> VariableList
   members. 

   @params os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NStructureDeclaration::print(std::ostream & os) const
{
  os << "Struct declared --- (" << ident << " {";

  for (VariableList::const_iterator it = members.begin(); it != members.end(); ++it)
      os << *(*it) << " ";
  
  os << "})";
  os << std::endl;
  
  return os;
}

/**
   Print function for NAssignmentStatement objects, which are inherited from the
   NStatement class. Similar to the NVariableDeclaration, this object is used
   after a variable has been declared, but is subsequently given a value in 
   another statement. For example, the second statement in,
      int a
      a = 4
   is the assignment statement.

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NAssignmentStatement::print(std::ostream & os) const
{
  os << "(Assignment: " << ident << " = " << expr << ")" << std::endl;
  return os;
}

/**
   Print function for NListAssignmentStatement objects, which are inherited from the
   NAssignmentStatement class. The NListAccess &list variable that is printed contains the 
   name (NIdentifier &ident) and the index (NExpression &index) of each element in
   the list. The expr value is the RHS of the expression in the assignment statement.
   (Not to be confused with a declaration statement.)

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NListAssignmentStatement::print(std::ostream & os) const
{
  os << "(Assignment: " << list << " = " << expr << "<" << std::endl;
  return os;
}

/**
   Print function for NStructureAssignmentStatement objects, which are inherited from
   the NAssignmentStatement class. The NStructure &structure variable that is printed
   contains the structure name (NIdentifier &ident) and its members (NIdentifier &member).
   The expr value is the RHS of the expression in the assignment statement. (Not to be 
   confused with a declaration statement.)

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NStructureAssignmentStatement::print(std::ostream & os) const
{
  os << "(Assignment: " << structure << " = " << expr << ")" << std::endl;
  return os;
}

/**
   Print function for NBinaryOperator objects, which are inherited from the NExpression
   class. This function prints the LHS, the binary operator symbol, followed by the RHS
   of the expression.

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NBinaryOperator::print(std::ostream & os) const
{
  std::string symbol;
  switch(op)
  {
  case TMUL:
      symbol = "*";
      break;
  case TADD:
      symbol = "+";
      break;
  case TDIV:
      symbol = "/";
      break;
  case TSUB:
      symbol = "-";
      break;
  case TMOD:
      symbol = "%";
      break;
  case TBAND:
      symbol = "&";
      break;
  case TBXOR:
      symbol = "^";
      break;
  case TBOR:
      symbol = "|";
      break;
  case TLNOT:
      symbol = "!";
      break;
  case TLOR:
      symbol = "||";
      break;
  case TLAND:
      symbol = "&&";
      break;
  case TCEQ:
      symbol = "==";
      break;
  case TCNEQ:
      symbol = "!=";
      break;
  case TCGT:
      symbol = ">";
      break;
  case TCLT:
      symbol = "<";
      break;
  case TCGE:
      symbol = ">=";
      break;
  case TCLE:
      symbol = "<=";
      break;
  default:
      symbol = "UNKNOWN OP";
      break;
      
  }
  os << "(BINOP: " << lhs << " " << symbol << " " << rhs << ")" << std::endl;
  return os;
}

/**
   Prints the name of a given NIdentifier object. (Note: the variable 'value' 
   is a string.)

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NIdentifier::print(std::ostream & os) const
{
  os << "Identifier: " << value;
  return os;
}

/**
   Prints an NListAccess object, which is a list element containing an NIdentifier &ident
   and an NExpression &index. (Note: an index may be more than just an integer, like a[1+2]
   would be the third element in a list.)

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NListAccess::print(std::ostream & os) const
{
  os << "(List access: " << ident << "[" << index << "])";
  return os;
}

/**
   Prints an NVariableAccess object, which is inherited from NExpression and contains an
   NIdentifier &ident member. The ident variable is the name of the variable being accessed.

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NVariableAccess::print(std::ostream & os) const
{
  os << "(Variable access: " << ident << ")";
  return os;
}

/** 
   Prints an NStructureAccess object, which is inherited from the NExpression class and contains both
   an NIdentifier &ident member (the struct name) and an NIdentifier &member member (the 
   struct's member name being accessed).

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NStructureAccess::print(std::ostream & os) const
{
  os << "(Struct access: " << ident << "." << member << ")";
  return os;
}

/**
   Prints an NReturn object, which is inherited from the NStatement class. The retExpr variable
   is of type NExpression.

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NReturn::print(std::ostream & os) const
{
  os << "(Return: " << retExpr << ")";
  os << std::endl;
  return os;
}

/**
   Prints an NDouble object, which is inherited from the NValue class. The variable 'value'
   that is printed is the number itself, not the string name of the identifier. 

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NDouble::print(std::ostream & os) const
{
  os << "DOUBLE:" << value;
  return os;
}

/**
   Prints an NBool object, which is inherited from the NValue class. The variable 'value'
   that is printed is 'true (1)' or 'false (0)', not the string name of the identifier. 

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NBool::print(std::ostream & os) const
{
    os << "BOOL: " << (value ? "true" : "false");
  return os;
}

/**
   Prints an NInt object, which is inherited from the NValue class. The variable 'value'
   that is printed is the number itself, not the string name of the identifier. 

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NInt::print(std::ostream & os) const
{
  os << "INT:" << value;
  return os;
}

/**
   Prints an NChar object, which is inherited from the NValue class. The variable 'value'
   that is printed is the number itself, not the string name of the identifier. 

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NChar::print(std::ostream & os) const
{
  os << "CHAR:" << value;
  return os;
}

/**
   Print function that allows for the genertic printing of an NValue object. The NValue
   class does not contain any variables, thus there are not any variables printed to the 
   output stream in this function.

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NValue::print(std::ostream & os) const
{
  os << "(Generic NValue)" << std::endl;
  return os;
}

/**
   Prints an NUInt object, which is inherited from the NValue class. The variable 'value'
   that is printed is the number itself, not the string name of the identifier. 

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NUInt::print(std::ostream & os) const
{
  os << "UINT:" << value;
  return os;
}

/**
   Prints an NString object, which is inherited from the NValue class. The variable 'value'
   that is printed is the string itself, not the string name of the identifier. 

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NString::print(std::ostream & os) const
{
  os << "STRING:" << value;
  return os;
}

/**
   Prints NLoopStatement objects, which are inherited from the NStatement class. The variable
   NIdentifer &list is the list variable name that is being looped through. NIdentifier &asVar
   is the temporary name inside of the loop block to reference the current list element. And
   the NBlock &loopBlock are the statements contained with in the braces of the loop statement.

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NLoopStatement::print(std::ostream & os) const
{
  os << "Loop: " << list << " as " << asVar << std::endl;
  os << "{" << loopBlock << "}" << std::endl;
  return os;
}

/**
   Prints NIfStatement objects, which are inherited from the NStatement class. There are four
   possible members that may be printed: 
    1) NExpression &condition, the 'if' condition in parentheses
    2) NBlock &thenblock, the series of statements that follow the 'if' condition
    3) NBlock *elseblock, a pointer to a statement or series of statements for 'else' conditions
    4) NStatement *elseif, a pointer to one statement completing the 'if' statement

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NIfStatement::print(std::ostream & os) const
{
  if (elseblock == NULL && elseif == NULL)
    os << "If: (" << condition << ") then " << thenblock << std::endl;
  if (elseblock != NULL && elseif == NULL)
    os << "If: (" << condition << ") then: " << thenblock << " else: " << *(elseblock) << std::endl;
  if (elseblock == NULL && elseif != NULL)
    {
      os << "If: (" << condition << ") then " << thenblock << std::endl;
      os << "Else if: " << *(elseif) << std::endl;
    }
  if (elseblock != NULL && elseif != NULL)
    {
      os << "If: (" << condition << ") then " << thenblock << std::endl;
      os << "Else if: " << *(elseif) << " else: " << *(elseblock) << std::endl;
    }
  return os;
}

/**
   Prints FunctionCall objects, which are inherited from the NExpression class. The 'args'
   variable is a typedef std::vector<NExpression *> ExpressionList and contains the arguments
   passed to the function. 

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NFunctionCall::print(std::ostream & os) const
{
  os << "Function Call: " << ident << std::endl;

  for(ExpressionList::const_iterator it = args.begin(); it != args.end(); ++it)
      os << *(*it) << std::endl;

  return os;
}

/**
   Prints NList objects, which are inherited from the NValue class. The 'value' variable
   is a typedef std::vector<NExpression *> ExpressionList and contains the values of the 
   list items, not the names of the items. 

   @param os Output stream to print to
   @return Output stream passed on
*/
std::ostream & NList::print(std::ostream & os) const
{
    os << "List: [";

    for (ExpressionList::const_iterator it = value.begin(); it != value.end(); ++it)
        os << *(*it) << " ";
    
    os << "]" << std::endl;
    return os;
}
