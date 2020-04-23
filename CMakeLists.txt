cmake_minimum_required(VERSION 3.15)

project(database_implementation)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

configure_file(catalog ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(test.cat ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(database_implementation_a42_test a42-test.cc
        BigQ.h BigQ.cc Pipe.cc Pipe.h
        Comparison.cc Comparison.h ComparisonEngine.cc ComparisonEngine.h
        DBFile.cc DBFile.h GenericDBFile.cc GenericDBFile.h Defs.h
        File.cc File.h Record.cc Record.h Schema.cc Schema.h
        TwoWayList.cc TwoWayList.h HeapDBFile.cc  HeapDBFile.h
        RelOp.cc RelOp.h Function.cc Function.h  SortedDBFile.cc SortedDBFile.h
        y.tab.o lex.yy.c RelOpStructs.h Statistics.cc Statistics.h)


