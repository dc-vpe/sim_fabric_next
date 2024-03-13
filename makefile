all:	bin/dsl.exe

CC=g++
ID = Includes
SD = Source
TS = tests/cpp
BUILD=-g

includes = 	$(ID)/parser.h  $(ID)/dsl_types.h $(ID)/Lexer.h $(ID)/utf8.h $(ID)/hashmap.h\
 			$(ID)/token.h $(ID)/U8String.h $(ID)/DSLValue.h $(ID)/KeyWords.h $(ID)/stack.h $(ID)/LocationInfo.h\
 			$(ID)/list.h $(ID)/ErrorProcessing.h $(ID)/ParseData.h $(ID)/cpu.h $(ID)/Collection.h $(ID)/JsonParser.h\
 			$(ID)/BinaryFileWriter.h $(ID)/BinaryFileReader.h $(ID)/SystemErrorHandlers.h $(ID)/SlotData.h\
 			$(ID)/ComponentData.h

sources = 	$(SD)/DSLValue.cpp $(SD)/lexer.cpp $(SD)/parser.cpp $(SD)/KeyWords.cpp $(SD)/token.cpp\
 			$(SD)/U8String.cpp $(SD)/ErrorProcessing.cpp $(SD)/cpu.cpp $(SD)/Collection.cpp $(SD)/main.cpp\
 			$(SD)/ParseData.cpp $(SD)/JsonParser.cpp $(SD)/BinaryFileWriter.cpp $(SD)/BinaryFileReader.cpp\
 			$(SD)/SlotData.cpp $(SD)/ComponentData.cpp

cpu_includes = 	$(ID)/dsl_types.h $(ID)/utf8.h $(ID)/hashmap.h $(ID)/U8String.h $(ID)/DSLValue.h $(ID)/LocationInfo.h\
 			$(ID)/list.h $(ID)/ErrorProcessing.h $(ID)/ParseData.h $(ID)/cpu.h $(ID)/Collection.h $(ID)/JsonParser.h\
 			$(ID)/BinaryFileWriter.h $(ID)/BinaryFileReader.h $(ID)/SystemErrorHandlers.h $(ID)/SlotData.h\
 			$(ID)/ComponentData.h

cpu_sources = 	$(SD)/DSLValue.cpp $(SD)/U8String.cpp $(SD)/ErrorProcessing.cpp $(SD)/cpu.cpp $(SD)/Collection.cpp\
 				$(SD)/dllmain.cpp $(SD)/ParseData.cpp $(SD)/JsonParser.cpp $(SD)/BinaryFileWriter.cpp\
 				$(SD)/BinaryFileReader.cpp $(SD)/SlotData.cpp $(SD)/ComponentData.cpp $(SD)/wcpu.cpp

bin/dsl.exe:	 $(sources) $(includes)
	$(CC) -o bin/dsl.exe $(BUILD) $(sources) -static-libgcc -static-libstdc++

bin/wcpu.dll:	$(cpu_sources) $(cpu_includes)
	$(CC) -c $(cpu_sources)
	$(CC) -shared -o bin/wcpu.dll $(cpu_sources)

clean:	bin/dsl.exe
	rm bin/dsl.exe
	rm bin/wcpu.dll
