GTEST_CFLAGS = `pkg-config --cflags Google_tests`
GTEST_LIBS = `pkg-config --libs Google_tests`
CC = g++ -O2 -Wno-deprecated
CLASSES = Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o GenericDBFile.o HeapDBFile.o SortedDBFile.o Pipe.o RelOp.o Function.o Statistics.o y.tab.o lex.yy.o yyfunc.tab.o lex.yyfunc.o
CLASSES_42 = QueryPlanner.o QueryPlannerNode.o Statistics.o Record.o Comparison.o Schema.o Function.o y.tab.o lex.yy.o
CLASSES_42_FOR_RULE = QueryPlanner.o QueryPlannerNode.o Statistics.o Record.o Comparison.o Schema.o Function.o y.tab_latest.o lex.yy_latest.o

tag = -i
test_out_tag = -ll

ifdef linux
tag = -n
test_out_tag = -lfl
endif

main: $(CLASSES) main.o
	$(CC) -o main $(CLASSES) $(PARSING) main.o $(test_out_tag) -lpthread -lgtest

##### GTests #####

HeapFileGTests.out: $(CLASSES) HeapFileGTests.o
	$(CC) -o HeapFileGTests.out $(CLASSES) HeapFileGTests.o $(test_out_tag) -lpthread -lgtest

BigQGTests.out: $(CLASSES) BigQGTests.o
	$(CC) -o BigQGTests.out $(CLASSES) BigQGTests.o $(test_out_tag) -lpthread -lgtest

QueryPlannerGTests.out: $(CLASSES_42_FOR_RULE) QueryPlannerGTests.o
	$(CC) -o QueryPlannerGTests.out $(CLASSES_42) QueryPlannerGTests.o $(test_out_tag) -lpthread -lgtest

SortedDBFileGTests.out : $(CLASSES) SortedDBFileGTests.o
	$(CC) -o SortedDBFileGTests.out $(CLASSES) SortedDBFileGTests.o $(test_out_tag) -lpthread -lgtest

RelOpGTests.out: $(CLASSES) RelOpGTests.o
	$(CC) -o RelOpGTests.out $(CLASSES) RelOpGTests.o $(test_out_tag) -lpthread -lgtest

##### Assignment Tests ####

a42.out: $(CLASSES_42_FOR_RULE) a42-test.o
	$(CC) -o a42.out $(CLASSES_42) a42-test.o $(test_out_tag)

a4-1.out: $(CLASSES) a41-test.o
	$(CC) -o a4-1.out $(CLASSES) a41-test.o $(test_out_tag) -lpthread

a3-test.out: $(CLASSES) a3-test.o
	$(CC) -o a3-test.out $(CLASSES) a3-test.o $(test_out_tag) -lpthread

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

QueryPlannerGTests.o: QueryPlannerGTests.cc
	$(CC) -g -c QueryPlannerGTests.cc

RelOpGTests.o: RelOpGTests.cc
	$(CC) -g -c RelOpGTests.cc

a42-test.o: a42-test.cc
	$(CC) -g -c a42-test.cc

a41-test.o: a41-test.cc
	$(CC) -g -c a41-test.cc

a2test.o: a2-test.cc
	$(CC) -g -c a2-test.cc

a1-test.o: a1-test.cc
	$(CC) -g -c a1-test.cc

a3-test.o: a3-test.cc
	$(CC) -g -c a3-test.cc

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

Statistics.o: Statistics.cc
	$(CC) -g -c Statistics.cc

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

QueryPlanner.o: QueryPlanner.cc
	$(CC) -g -c QueryPlanner.cc

QueryPlannerNode.o: QueryPlannerNode.cc
	$(CC) -g -c QueryPlannerNode.cc

y.tab_latest.o: Parser_Latest.y
	yacc -d Parser_Latest.y
	sed $(tag) -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" y.tab.c
	g++ -c y.tab.c

lex.yy_latest.o: Lexer_Latest.l
	lex  Lexer_Latest.l
	gcc  -c lex.yy.c
	
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
	rm -f *tmp*.*
	rm -f *.bin*