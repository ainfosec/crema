/**
   @file types.h
   @brief Header file for type classes used to perform type-checking during the semantic analysis checks
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   The Type family of classes are used to store type information about various expressions and
   AST nodes to assist in the analysis of typing during semantic analysis.
*/
#ifndef CREMA_TYPE_H_
#define CREMA_TYPE_H_

#include <iostream>

/**
 *  Root type class */
class Type {
public:
    virtual ~Type() { }
    virtual std::ostream & print(std::ostream & os) const { }
    friend std::ostream & operator<<(std::ostream & os, Type & type); 
};

#endif // CREMA_TYPE_H_
