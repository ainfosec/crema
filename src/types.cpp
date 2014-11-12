/**
   @file types.cpp
   @brief Contains type class implementation
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   This file contains the function implementations for all type-related
   semantic checks.
*/

#include "types.h"
#include "ast.h"
#include "parser.h"

/**
   Equality operator for Types

   @param t1 First Type to compare against
   @param t2 Second Type to compare against
   @return true if the types are the same in type and dimension, false otherwise
*/
bool operator==(Type & t1, Type & t2)
{
    return (t1.typecode == t2.typecode) && (t1.isList == t2.isList);
}

/**
   Non-equality operator for Types

   @param t1 First Type to compare against
   @param t2 Second Type to compare against
   @return false if the types are the same in type and dimension, true otherwise
*/
bool operator!=(Type & t1, Type & t2)
{
    return !(t1 == t2);
}

/**
   Greater than operator for Types

   One Type is considered "larger" than another if it can upcast.
   E.g. DOUBLE > INT as integers can be upcast to doubles, but not vice versa

   @param t1 First Type to compare
   @param t2 Second Type to compare
   @return true is t1 is "larger" than t2, false otherwise
*/
bool operator>(Type & t1, Type & t2)
{
    // List types go with list types, singtons with singletons
    if (t1.isList != t2.isList)
    {
	return false;
    }
    
    // Double > Int
    if (t1.typecode == DOUBLE && (t2.typecode == INT || t2.typecode == UINT))
    {
	return true;
    }

    // Double/Int > Bool (true = 1, false = 0)
    if ((t1.typecode == INT || t2.typecode == UINT || t2.typecode == DOUBLE) && t2.typecode == BOOL)
    {
	return true;
    }

    // Bool > Double/Int
    if (t1.typecode == BOOL && (t2.typecode == DOUBLE || t2.typecode == INT || t2.typecode == UINT))
    {
	return true;
    }

    // String > Double/Int
    if (t1.typecode == STRING && (t2.typecode == UINT || t2.typecode == INT || t2.typecode == DOUBLE))
    {
	return true;
    }
    
    return false;
}

/**
   Less than operator for Types

   One Type is considered "larger" than another if it can upcast.
   E.g. DOUBLE >= INT as integers can be upcast to doubles, but not vice versa

   @param t1 First Type to compare
   @param t2 Second Type to compare
   @return true is t1 is "smaller" than to t2, false otherwise
*/
bool operator<(Type & t1, Type & t2)
{
    return !(t1 >= t2);
}

/**
   Greater than or equal operator for Types

   One Type is considered "larger" than another if it can upcast.
   E.g. DOUBLE >= INT as integers can be upcast to doubles, but not vice versa

   @param t1 First Type to compare
   @param t2 Second Type to compare
   @return true is t1 is "larger" than or equal to t2, false otherwise
*/
bool operator>=(Type & t1, Type & t2)
{
    return (t1 == t2) || (t1 > t2);
}

/**
   Less than or equal operator for Types

   One Type is considered "larger" than another if it can upcast.
   E.g. DOUBLE >= INT as integers can be upcast to doubles, but not vice versa

   @param t1 First Type to compare
   @param t2 Second Type to compare
   @return true is t1 is "smaller" than or equal to t2, false otherwise
*/
bool operator<=(Type & t1, Type & t2)
{
    return (t1 == t2) || (t1 < t2);
}



/**
   Pretty-prints a Type object

   @param os Stream to print to
   @param type Type to print
   @return Stream printed to
*/
std::ostream & operator<<(std::ostream & os, Type & type)
{
    type.print(os);
    return os;
}

std::ostream & Type::print(std::ostream & os) const
{
    switch(typecode)
    {
    case INT:
	os << "INT";
	break;
    case UINT:
	os << "UINT";
	break;
    case DOUBLE:
	os << "DOUBLE";
	break;
    case STRING:
	os << "STRING";
	break;
    case BOOL:
	os << "BOOL";
	break;
    case VOID:
	os << "VOID";
	break;
    case STRUCT:
	os << "STRUCT";
	break;
    case INVALID:
	os << "INVALID";
	break;
    default:
	os << "UNKNOWN TYPE!";
	break;
    }
    if (isList)
    {
	os << "[]";
    }
    return os;
}

std::ostream & StructType::print(std::ostream & os) const
{
    os << "STRUCT " << ident;
    return os;
}

/**
   Function to convert a Crema Type to an llvm::Type

   @return LLVM type object corresponding with the Type
*/
llvm::Type * Type::toLlvmType()
{
    switch(typecode)
    {
    case INT:
	return llvm::Type::getInt64Ty(llvm::getGlobalContext());
	break;
    case DOUBLE:
	return llvm::Type::getDoubleTy(llvm::getGlobalContext());
	break;
    case VOID:
	return llvm::Type::getVoidTy(llvm::getGlobalContext());
	break;
    case BOOL:
	return llvm::Type::getInt8Ty(llvm::getGlobalContext());
	break;
    default:
	return NULL;
	break;
    }
    return NULL;
}

/**
   Private function to set the typecode of a Type based on the Flex/Bison enumeration

   @param type Token value for type
*/
void Type::setType(int type)
{
    switch(type)
    {
    case TTINT:
	typecode = INT;
	break;
    case TTUINT:
	typecode = UINT;
	break;
    case TTDOUBLE:
	typecode = DOUBLE;
	break;
    case TTBOOL:
	typecode = BOOL;
	break;
    case TTSTR:
	typecode = STRING;
	break;
    case TTVOID:
	typecode = VOID;
	break;
    }
}
