all:
	flex lexer.l
	bison parser.y
	g++ crema.cpp

clean:
	-rm *~ *.o cremacc
