cmake_minimum_required(VERSION 3.15)

project(database_implementation)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

configure_file(catalog ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(database_implementation main.cc
        Comparison.cc Comparison.h ComparisonEngine.cc ComparisonEngine.h
        DBFile.cc DBFile.h GenericDBFile.cc GenericDBFile.h Defs.h
        File.cc File.h Record.cc Record.h Schema.cc Schema.h
        TwoWayList.cc TwoWayList.h HeapDBFile.cc  HeapDBFile.h
        RelOp.cc RelOp.h Function.cc Function.h  SortedDBFile.cc SortedDBFile.h
        y.tab.o yyfunc.tab.o lex.yy.c lex.yyfunc.c
        )

add_executable(database_implementation_a1_test a1-test.cc a1-test.h
        Comparison.cc Comparison.h ComparisonEngine.cc ComparisonEngine.h
        DBFile.cc DBFile.h GenericDBFile.cc GenericDBFile.h Defs.h
        File.cc File.h Record.cc Record.h Schema.cc Schema.h
        TwoWayList.cc TwoWayList.h HeapDBFile.cc  HeapDBFile.h
        RelOp.cc RelOp.h Function.cc Function.h  SortedDBFile.cc SortedDBFile.h
        y.tab.o yyfunc.tab.o lex.yy.c lex.yyfunc.c
        )

add_executable(database_implementation_a2_p1_test a2-test.cc a2-test.h
        Comparison.cc Comparison.h ComparisonEngine.cc ComparisonEngine.h
        DBFile.cc DBFile.h GenericDBFile.cc GenericDBFile.h Defs.h
        File.cc File.h Record.cc Record.h Schema.cc Schema.h
        TwoWayList.cc TwoWayList.h HeapDBFile.cc  HeapDBFile.h
        RelOp.cc RelOp.h Function.cc Function.h  SortedDBFile.cc SortedDBFile.h
        y.tab.o yyfunc.tab.o lex.yy.c lex.yyfunc.c
        )

add_executable(database_implementation_a2_p2_test a2-2test.cc a2-2test.h
        Comparison.cc Comparison.h ComparisonEngine.cc ComparisonEngine.h
        DBFile.cc DBFile.h GenericDBFile.cc GenericDBFile.h Defs.h
        File.cc File.h Record.cc Record.h Schema.cc Schema.h
        TwoWayList.cc TwoWayList.h HeapDBFile.cc  HeapDBFile.h
        RelOp.cc RelOp.h Function.cc Function.h  SortedDBFile.cc SortedDBFile.h
        y.tab.o yyfunc.tab.o lex.yy.c lex.yyfunc.c
        )

add_executable(database_implementation_a3_test test.cc test.h
        Comparison.cc Comparison.h ComparisonEngine.cc ComparisonEngine.h
        DBFile.cc DBFile.h GenericDBFile.cc GenericDBFile.h Defs.h
        File.cc File.h Record.cc Record.h Schema.cc Schema.h
        TwoWayList.cc TwoWayList.h HeapDBFile.cc  HeapDBFile.h
        RelOp.cc RelOp.h Function.cc Function.h  SortedDBFile.cc SortedDBFile.h
        y.tab.o yyfunc.tab.o lex.yy.c lex.yyfunc.c
        )


