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
    return (t1.typecode == t2.typecode) && (t1.list == t2.list);
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
    default:
	os << "UNKNOWN TYPE!";
	break;
    }
    return os;
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
