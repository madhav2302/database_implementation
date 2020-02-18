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
public:
    BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);

    ~BigQ();
};

struct thread_data {
    Pipe &in;
    Pipe &out;
    OrderMaker &sortorder;
    int runlen;
};

void *TPMMS(void *data);

std::string Init(File *file);

void Phase1(File *file, Pipe &in, OrderMaker &sortorder, int runlen);

void Phase2(File *file, Pipe &out, OrderMaker &sortorder, int runlen);

void Finish(File *file, std::string fileName, Pipe &out);

int SortSingleRunData(File *file, vector<Page *> &pages, OrderMaker *sortorder, int runlen, int writePage);

class RecordWrapper;

class SingleRun {
private:
    int startPage;
    int endPage;

    Page *page = new Page();
    File *file;
public:
    SingleRun(File *file, int startPage, int endPage);

    ~SingleRun();

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
    OrderMaker *orderMaker;
    ComparisonEngine comp;

    CustomRecordCompare(OrderMaker *orderMaker) {
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
