all:	bin/dsl.exe
CC=g++
ID = Includes
SD = Source
TS = tests
BUILD=-g

includes = 	$(ID)/parser.h  $(ID)/dsl_types.h $(ID)/Lexer.h $(ID)/utf8.h $(ID)/hashmap.h\
 			$(ID)/token.h $(ID)/U8String.h $(ID)/DSLValue.h $(ID)/KeyWords.h $(ID)/stack.h $(ID)/LocationInfo.h\
 			$(ID)/list.h $(ID)/ErrorProcessing.h $(ID)/ParseData.h $(ID)/cpu.h $(ID)/Collection.h $(ID)/JsonParser.h

sources = 	$(SD)/DSLValue.cpp $(SD)/lexer.cpp $(SD)/parser.cpp $(SD)/KeyWords.cpp $(SD)/token.cpp\
 			$(SD)/U8String.cpp $(SD)/ErrorProcessing.cpp $(SD)/cpu.cpp $(SD)/Collection.cpp $(SD)/main.cpp\
 			$(SD)/ParseData.cpp $(SD)/JsonParser.cpp

bin/dsl.exe:	 $(sources) $(includes)
	$(CC) -o bin/dsl.exe $(BUILD) $(sources) -static-libgcc -static-libstdc++

clean:	bin/dsl.exe
	rm bin/dsl.exe
