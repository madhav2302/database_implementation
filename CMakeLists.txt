cmake_minimum_required(VERSION 3.15)

project(database_implementation)

set(CMAKE_CXX_STANDARD 11)

add_executable(database_implementation main.cc
        Comparison.cc
        Comparison.h
        ComparisonEngine.cc
        ComparisonEngine.h
        DBFile.cc
        DBFile.h
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
        )

configure_file(catalog ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)