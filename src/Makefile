all:
	bison -d -o parser.cpp parser.y
	flex -olexer.cpp lexer.l
	g++ -c `llvm-config --cppflags` parser.cpp lexer.cpp crema.cpp

clean:
	-rm *~ *.o cremacc parser.cpp parser.h lexer.cpp
