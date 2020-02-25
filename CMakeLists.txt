cmake_minimum_required(VERSION 3.15)

project(database_implementation)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(database_implementation main.cc
        Comparison.cc
        Comparison.h
        ComparisonEngine.cc
        ComparisonEngine.h
        DBFile.cc
        DBFile.h
        GenericDBFile.cc
        GenericDBFile.h
        Defs.h
        File.cc
        File.h
        Record.cc
        Record.h
        Schema.cc
        Schema.h
        TwoWayList.cc
        TwoWayList.h
        lex.yy.c
        y.tab.o
        HeapDBFile.cc SortedDBFile.cc HeapDBFile.h SortedDBFile.h)

configure_file(catalog ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(database_implementation_a1_test a1-test.cc
        a1-test.h
        Comparison.cc
        Comparison.h
        ComparisonEngine.cc
        ComparisonEngine.h
        DBFile.cc
        DBFile.h
        GenericDBFile.cc
        GenericDBFile.h
        Defs.h
        File.cc
        File.h
        Record.cc
        Record.h
        Schema.cc
        Schema.h
        TwoWayList.cc
        TwoWayList.h
        lex.yy.c
        y.tab.o HeapDBFile.cc SortedDBFile.cc HeapDBFile.h SortedDBFile.h)

add_executable(database_implementation_a2_p1_test a2-test.cc
        a2-test.h
        BigQ.cc
        BigQ.h
        Pipe.cc
        Pipe.h
        Comparison.cc
        Comparison.h
        ComparisonEngine.cc
        ComparisonEngine.h
        DBFile.cc
        DBFile.h
        GenericDBFile.cc
        GenericDBFile.h
        Defs.h
        File.cc
        File.h
        Record.cc
        Record.h
        Schema.cc
        Schema.h
        TwoWayList.cc
        TwoWayList.h
        lex.yy.c
        y.tab.o
        HeapDBFile.cc SortedDBFile.cc HeapDBFile.h SortedDBFile.h)

add_executable(database_implementation_a2_p2_test test.cc
        test.h
        BigQ.cc
        BigQ.h
        Pipe.cc
        Pipe.h
        Comparison.cc
        Comparison.h
        ComparisonEngine.cc
        ComparisonEngine.h
        DBFile.cc
        DBFile.h
        GenericDBFile.cc
        GenericDBFile.h
        Defs.h
        File.cc
        File.h
        Record.cc
        Record.h
        Schema.cc
        Schema.h
        TwoWayList.cc
        TwoWayList.h
        lex.yy.c
        y.tab.o
        HeapDBFile.cc SortedDBFile.cc HeapDBFile.h SortedDBFile.h)


