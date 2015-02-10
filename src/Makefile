CC := g++ #clang++

DEP_FILES := parser.cpp lexer.cpp ast.cpp codegen.cpp types.cpp semantics.cpp crema.cpp
OBJ_FILES := parser.o lexer.o ast.o codegen.o types.o semantics.o crema.o
CPP_FLAGS := `llvm-config --cxxflags` -Wno-cast-qual -std=c++11 -g
LD_FLAGS := `llvm-config --ldflags`
LIBS := `llvm-config --libs core jit native interpreter`

all: cremacc

cremacc: parser.o lexer.o ast.o types.o crema.o codegen.o semantics.o 
	$(CC) -std=c++11 -o cremacc $(OBJ_FILES) $(LIBS) $(LD_FLAGS)

parser.o: parser.h
	$(CC) -c $(CPP_FLAGS) parser.cpp

parser.h:
	bison -d -o parser.cpp --defines=parser.h parser.y

lexer.o: lexer.cpp
	$(CC) -c $(CPP_FLAGS) lexer.cpp

lexer.cpp:
	flex -olexer.cpp lexer.l

ast.o:
	$(CC) -c $(CPP_FLAGS) ast.cpp

types.o:
	$(CC) -c $(CPP_FLAGS) types.cpp

codegen.o:
	$(CC) -c $(CPP_FLAGS) codegen.cpp

crema.o:
	$(CC) -c $(CPP_FLAGS) crema.cpp 

semantics.o:
	$(CC) -std=c++11 -c $(CPP_FLAGS) semantics.cpp

graph:
	bison -d -o parser.cpp --defines=parser.h -g parser.y
	dot -Tpng parser.dot > parser.png

doc: clean
	cd ../docs/doxygen && doxygen

clean:
	-rm *~ *.o cremacc parser.cpp parser.h lexer.cpp
