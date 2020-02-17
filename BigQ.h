#ifndef BIGQ_H
#define BIGQ_H

#include <pthread.h>
#include <iostream>
#include "Pipe.h"
#include "File.h"
#include "Record.h"
#include <vector>

using namespace std;

class BigQ {

private:
    File *file = new File();
    Page *page;
    off_t writePage;


    void SortRun(vector<Page *> &pages, Pipe &in, Pipe &out, OrderMaker *sortorder, int runlen);

public:

    BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);

    ~BigQ();
};

class RecordWrapper;

class Run {
private:
    int startPage;
    int endPage;

    Page *page = new Page();
    File *file;
public:
    Run(File *file, int startPage, int endPage);

    ~Run();

    int GetFirst(RecordWrapper *firstOne);
};

class RecordWrapper {
public:
    Record *firstOne = new Record();
    OrderMaker *sortorder;
    int runArrayIndex;

    RecordWrapper(OrderMaker *sortorder, int runArrayIndex) {
        this->sortorder = sortorder;
        this->runArrayIndex = runArrayIndex;
    }
};

#endif
