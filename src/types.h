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
 *  Enumeration of types available to Crema programs */
enum TypeCodes {
    INT,
    DOUBLE,
    STRING,
    VOID,
    BOOL,
    UINT,
    INVALID
};

/**
 *  Root type class */
class Type {
private:
    void setType(int type);
public:
    bool list;
    TypeCodes typecode; /**< typecode for the Type */
    virtual ~Type() { }
Type() : typecode(INVALID) { }
    Type(int type) { setType(type); list = false; }
    Type(int type, bool list) { setType(type); list = list; }
    bool isList() { return list; }
    std::ostream & print(std::ostream & os) const;
    friend std::ostream & operator<<(std::ostream & os, Type & type); 
    friend bool operator==(Type & t1, Type & t2);
    friend bool operator!=(Type & t1, Type & t2);
    friend bool operator>(Type & t1, Type & t2);
    friend bool operator>=(Type & t1, Type & t2);
    friend bool operator<(Type & t1, Type & t2);
    friend bool operator<=(Type & t1, Type & t2);
};

#endif // CREMA_TYPE_H_
