GTEST_CFLAGS = `pkg-config --cflags Google_tests`
GTEST_LIBS = `pkg-config --libs Google_tests`
CC = g++ -O2 -Wno-deprecated
CLASSES = Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o GenericDBFile.o HeapDBFile.o SortedDBFile.o Pipe.o RelOp.o Function.o y.tab.o lex.yy.o yyfunc.tab.o lex.yyfunc.o

tag = -i
test_out_tag = -ll

ifdef linux
tag = -n
test_out_tag = -lfl
endif

main: $(CLASSES) main.o
	$(CC) -o main $(CLASSES) main.o $(test_out_tag) -lpthread -lgtest

##### GTests #####

HeapFileGTests.out: $(CLASSES) HeapFileGTests.o
	$(CC) -o HeapFileGTests.out $(CLASSES) HeapFileGTests.o $(test_out_tag) -lpthread -lgtest

BigQGTests.out: $(CLASSES) BigQGTests.o
	$(CC) -o BigQGTests.out $(CLASSES) BigQGTests.o $(test_out_tag) -lpthread -lgtest

SortedDBFileGTests.out : $(CLASSES) SortedDBFileGTests.o
	$(CC) -o SortedDBFileGTests.out $(CLASSES) SortedDBFileGTests.o $(test_out_tag) -lpthread -lgtest

##### Assignment Tests ####

test.out: $(CLASSES) test.o
	$(CC) -o test.out $(CLASSES) test.o $(test_out_tag) -lpthread

a2-2test.out: $(CLASSES) a2-2test.o
	$(CC) -o a2-2test.out $(CLASSES) a2-2test.o $(test_out_tag) -lpthread

a2test.out: $(CLASSES) a2-test.o
	$(CC) -o a2test.out $(CLASSES) a2-test.o $(test_out_tag) -lpthread

a1test.out: $(CLASSES) a1-test.o
	$(CC) -o a1test.out $(CLASSES) a1-test.o $(test_out_tag) -lpthread

HeapFileGTests.o: HeapFileGTests.cc
	$(CC) -g -c HeapFileGTests.cc

BigQGTests.o: BigQGTests.cc
	$(CC) -g -c BigQGTests.cc

a2test.o: a2-test.cc
	$(CC) -g -c a2-test.cc

a1-test.o: a1-test.cc
	$(CC) -g -c a1-test.cc

test.o: test.cc
	$(CC) -g -c test.cc

a2-2test.o: a2-2test.cc
	$(CC) -g -c a2-2test.cc

main.o: main.cc
	$(CC) -g -c main.cc
	
Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc
	
ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc
	
DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

GenericDBFile.o: GenericDBFile.cc
	$(CC) -g -c GenericDBFile.cc

HeapDBFile.o: HeapDBFile.cc
	$(CC) -g -c HeapDBFile.cc

SortedDBFile.o: SortedDBFile.cc
	$(CC) -g -c SortedDBFile.cc

File.o: File.cc
	$(CC) -g -c File.cc

Pipe.o: Pipe.cc
	$(CC) -g -c Pipe.cc

BigQ.o: BigQ.cc
	$(CC) -g -c BigQ.cc

RelOp.o: RelOp.cc
	$(CC) -g -c RelOp.cc

Function.o: Function.cc
	$(CC) -g -c Function.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc
	
y.tab.o: Parser.y
	yacc -d Parser.y
	# sed $(tag) -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" y.tab.c
	g++ -c y.tab.c
		
yyfunc.tab.o: ParserFunc.y
	yacc -p "yyfunc" -b "yyfunc" -d ParserFunc.y
	#sed $(tag) yyfunc.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c yyfunc.tab.c
	
lex.yy.o: Lexer.l
	lex  Lexer.l
	gcc  -c lex.yy.c

lex.yyfunc.o: LexerFunc.l
	lex -Pyyfunc LexerFunc.l
	gcc  -c lex.yyfunc.c


clean: 
	rm -f *.o
	rm -f *.out
	rm -f y.tab.*
	rm -f yyfunc.tab.*
	rm -f lex.yy.*
	rm -f lex.yyfunc*
	rm -f main
	rm -f *tmp*.bin
	rm -f *.bin*