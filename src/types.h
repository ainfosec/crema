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
    CHAR,
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
    bool isList; /**< Bool value if the type is a list-type */
    bool isStruct; /**< Bool value if the type is a struct type */
    TypeCodes typecode; /**< typecode for the Type */
    virtual ~Type() { }
Type() : typecode(INVALID) { }
    Type(int type) { isList = false; setType(type); isStruct = false; }
    Type(int type, bool l) { setType(type); isList = l; isStruct = false; }
    Type(Type & t, bool l) { typecode = t.typecode; isList = l; isStruct = false; }
    bool getIsList() { return isList; }
    size_t getSize();
    llvm::Type * toLlvmType();
    static Type & getLargerType(Type & t1, Type & t2);
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
StructType(NIdentifier & ident) : ident(ident) { setType(STRUCT); isStruct = true; }
    std::ostream & print(std::ostream & os) const;
};

#endif // CREMA_TYPE_H_
