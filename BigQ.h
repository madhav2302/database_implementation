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
    int runArrayIndex;

    RecordWrapper(int runArrayIndex) {
        this->runArrayIndex = runArrayIndex;
    }
};

struct CustomRecordCompare {
    OrderMaker* orderMaker;
    ComparisonEngine comp;

    CustomRecordCompare(OrderMaker* orderMaker) {
        this->orderMaker = orderMaker;
    }

    bool operator()(Record *lhs, Record *rhs) {
        return comp.Compare(lhs, rhs, this->orderMaker) > 0;
    }

    bool operator()(RecordWrapper *lhs, RecordWrapper *rhs) {
        return comp.Compare(lhs->firstOne, rhs->firstOne, this->orderMaker) > 0;
    }
};

static std::string randomFileName() {
    time_t result = time(nullptr);
    std::stringstream ss;
    ss << result;
    return "tmp" + ss.str() + ".bin";
}

#endif
