CC := g++ #clang++

all:
	bison -d -o parser.cpp --defines=parser.h parser.y
	flex -olexer.cpp lexer.l
	$(CC) -o cremacc `llvm-config --cppflags` parser.cpp lexer.cpp ast.cpp semantics.cpp crema.cpp

graph:
	bison -d -o parser.cpp --defines=parser.h -g parser.y
	dot -Tpng parser.dot > parser.png

clean:
	-rm *~ *.o cremacc parser.cpp parser.h lexer.cpp
