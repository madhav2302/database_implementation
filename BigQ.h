#ifndef BIGQ_H
#define BIGQ_H

#include <pthread.h>
#include <sstream>
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
    off_t writePage{};

    void SortRun(vector<Page *> &pages, Pipe &out, OrderMaker *sortorder, int runlen);

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
    ComparisonEngine *comp;
    int runArrayIndex;

    RecordWrapper(OrderMaker *sortorder, ComparisonEngine *comp, int runArrayIndex) {
        this->sortorder = sortorder;
        this->comp = comp;
        this->runArrayIndex = runArrayIndex;
    }
};

struct CustomCompare {
    bool operator()(RecordWrapper *lhs, RecordWrapper *rhs) {
        return lhs->comp->Compare(lhs->firstOne, rhs->firstOne, lhs->sortorder) != 1;
    }
};

static std::string randomFileName() {
    time_t result = time(nullptr);
    std::stringstream ss;
    ss << result;
    return "tmp" + ss.str() + ".bin";
}

#endif
