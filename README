![Crema-rado.png](https://bitbucket.org/repo/arMj8M/images/192944102-Crema-rado.png)
# Crema
(C) 2014 Assured Information Security, Inc.

## Introduction
Crema is a LLVM front-end that aims to specifically execute in sub-Turing Complete space.
Designed to be simple to learn, and practical for the majority of programming tasks needed,
Crema can restrict the computational complexity of the program to the minimum needed to improve
security.

## Technical Details
Crema is developed in C++ in order to natively utilize the LLVM tool-chain and integrate with
flex/bison to simplify parser generation.

## Getting started
Start off by cloning the latest version of Crema from this git repository

### Building the Crema Tool
Before building the Crema tool, make sure that you have the following dependencies installed on your environment:

1. g++
2. bison: 		http://www.gnu.org/software/bison/
3. llvm-dev
4. flex:		http://flex.sourceforge.net/

Build the Crema tool by running ```make``` in the src directory. This will create the "cremacc" program. You can clean the src directory by running ```make clean```

### Compiling your Crema programs with cremacc
To compile a Crema program into LLVM IR for JIT execution, simply use the "cremacc" program.

Crema files use the extention ".crema". To build a crema file into an executable named "program.exe", use the command:
```
./cremacc -f <path-to-a-crema-file> -o program
```

To print LLVM assembly to a file, use the -S option:
``` ./cremacc -f <path-to-a-crema-file> -S <output-file-name>.ll```

For help with all of the other command line options available for cremacc, simply run:
```./cremacc -h```

## Learning Crema
Use this repository's Wiki page to get started with code examples, reference information, and other general topics about the Crema language: https://bitbucket.org/aistorreyj/crema/wiki/Home