CC := g++ #clang++

DEP_FILES := parser.cpp lexer.cpp ast.cpp codegen.cpp types.cpp semantics.cpp
CPP_FLAGS := `llvm-config --cxxflags`
LD_FLAGS := `llvm-config --ldflags`
LIBS := `llvm-config --libs core jit native`

all:
	bison -d -o parser.cpp --defines=parser.h parser.y
	flex -olexer.cpp lexer.l
	$(CC) -std=c++11 -o cremacc $(CPP_FLAGS) $(DEP_FILES) crema.cpp $(LIBS) $(LD_FLAGS)

graph:
	bison -d -o parser.cpp --defines=parser.h -g parser.y
	dot -Tpng parser.dot > parser.png

doc: clean
	cd ../docs/doxygen && doxygen

clean:
	-rm *~ *.o cremacc parser.cpp parser.h lexer.cpp
