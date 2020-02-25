GTEST_CFLAGS = `pkg-config --cflags Google_tests`
GTEST_LIBS = `pkg-config --libs Google_tests`
CC = g++ -O2 -Wno-deprecated
CLASSES = Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o GenericDBFile.o HeapDBFile.o SortedDBFile.o Pipe.o y.tab.o lex.yy.o

tag = -i
test_out_tag = -ll

ifdef linux
tag = -n
test_out_tag = -lfl
endif

HeapFileGTests.out: $(CLASSES) HeapFileGTests.o
	$(CC) -o HeapFileGTests.out $(CLASSES) HeapFileGTests.o $(test_out_tag) -lpthread -lgtest

BigQTests.out: $(CLASSES) BigQTests.o
	$(CC) -o BigQTests.out $(CLASSES) BigQTests.o $(test_out_tag) -lpthread -lgtest

test.out: $(CLASSES) test.o
	$(CC) -o test.out $(CLASSES) test.o $(test_out_tag) -lpthread

a2test.out: $(CLASSES) a2-test.o
	$(CC) -o a2test.out $(CLASSES) a2-test.o $(test_out_tag) -lpthread

a1test.out: $(CLASSES) a1-test.o
	$(CC) -o a1test.out $(CLASSES) a1-test.o $(test_out_tag)

HeapFileGTests.o: HeapFileGTests.cc
	$(CC) -g -c HeapFileGTests.cc

BigQTests.o: BigQTests.cc
	$(CC) -g -c BigQTests.cc

test.o: test.cc
	$(CC) -g -c test.cc


a2test.o: a2-test.cc
	$(CC) -g -c a2-test.cc

a1-test.o: a1-test.cc
	$(CC) -g -c a1-test.cc

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

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc
	
y.tab.o: Parser.y
	yacc -d Parser.y
	sed $(tag) -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" y.tab.c
	g++ -c y.tab.c

lex.yy.o: Lexer.l
	lex  Lexer.l
	gcc  -c lex.yy.c

clean: 
	rm -f *.o
	rm -f *.out
	rm -f y.tab.c
	rm -f y.tab.c-e
	rm -f lex.yy.c
	rm -f y.tab.h
	rm -f main
	rm -f *.bin.bigq
	rm -f tmp*.bin