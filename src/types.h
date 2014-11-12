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
#include <llvm/IR/Type.h>

class NIdentifier;

/**
 *  Enumeration of types available to Crema programs */
enum TypeCodes {
    INT,
    DOUBLE,
    STRING,
    VOID,
    BOOL,
    UINT,
    STRUCT,
    INVALID
};

/**
 *  Root type class */
class Type {
protected:
    void setType(int type);
public:
<<<<<<< HEAD
    bool list; /**< Bool value if the type is a list-type */
    bool structt; /**< Bool value if the type is a struct type */
    TypeCodes typecode; /**< typecode for the Type */
    virtual ~Type() { }
Type() : typecode(INVALID) { }
    Type(int type) { setType(type); list = false; structt = false; }
    Type(int type, bool l) { setType(type); list = l; structt = false; }
    Type(Type & t, bool l) { typecode = t.typecode; list = l; structt = false; }
    bool isList() { return list; }
=======
    bool isList; /**< Bool value if the type is a list-type */
    TypeCodes typecode; /**< typecode for the Type */
    virtual ~Type() { }
Type() : typecode(INVALID) { }
    Type(int type) { setType(type); isList = false; }
    Type(int type, bool l) { setType(type); isList = l; }
    Type(Type & t, bool l) { typecode = t.typecode; isList = l; }
    bool getIsList() { return isList; }
>>>>>>> Changed 'bool list' var to 'bool isList' and 'bool isList' function to 'bool getIsList'
    llvm::Type * toLlvmType();
    virtual std::ostream & print(std::ostream & os) const;
    friend std::ostream & operator<<(std::ostream & os, Type & type); 
    friend bool operator==(Type & t1, Type & t2);
    friend bool operator!=(Type & t1, Type & t2);
    friend bool operator>(Type & t1, Type & t2);
    friend bool operator>=(Type & t1, Type & t2);
    friend bool operator<(Type & t1, Type & t2);
    friend bool operator<=(Type & t1, Type & t2);
};

class StructType : public Type
{
public:
    NIdentifier & ident;
StructType(NIdentifier & ident) : ident(ident) { setType(STRUCT); structt = true; }
    std::ostream & print(std::ostream & os) const;
};

#endif // CREMA_TYPE_H_
