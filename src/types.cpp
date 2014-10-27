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

std::ostream & operator<<(std::ostream & os, Type & type) {
    type.print(os);
    return os;
}
