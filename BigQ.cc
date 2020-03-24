#include <vector>
#include <cmath>
#include <queue>
#include <algorithm>
#include <cstring>
#include "BigQ.h"
#include "File.h"

BigQ::BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen) {
    thread_data *data = new thread_data(in, out, sortorder, runlen);

    pthread_t worker_t;
    pthread_create(&worker_t, NULL, TPMMS, (void *) data);
}

void *TPMMS(void *data) {
    thread_data *threadData = (thread_data *) data;

    File *file = new File();
    std::string fileName = Init(file);

    Phase1(file, threadData->in, threadData->sortorder, threadData->runlen);
    Phase2(file, threadData->out, threadData->sortorder, threadData->runlen);
    Finish(file, fileName, threadData->out);

    delete file;
    return nullptr;
}

std::string Init(File *file) {
    std::string fileName("bigq_" + randomFileName());
    char cstr[fileName.size() + 1];
    strcpy(cstr, fileName.c_str());
    file->Open(0, cstr);
    return fileName;
}

void Phase1(File *file, Pipe &in, OrderMaker &sortorder, int runlen) {
    Page *page = new Page();
    int writePage = 0;

    Record *temp = new Record();
    vector<Page *> pages;

    while (in.Remove(temp)) {
        if (!page->Append(temp)) {
            pages.push_back(page);

            if (pages.size() == runlen) {
                writePage = SortSingleRunData(file, pages, &sortorder, runlen, writePage);

                if (!pages.empty()) {
                    page = pages.back();
                    pages.pop_back();
                } else {
                    page = new Page();
                }
            } else {
                page = new Page();
            }

            // TODO :: Handle case when page popped back from pages after sorting single run is full
            // It will miss current record
            page->Append(temp);
        }

        temp = new Record();
    }

    // Push final page
    pages.push_back(page);

    // Sorting remaining records as run
    while (!pages.empty()) writePage = SortSingleRunData(file, pages, &sortorder, runlen, writePage);
}

int SortSingleRunData(File *file, vector<Page *> &pages, OrderMaker *sortorder, int runlen, int writePage) {
    Record *temp = new Record();

    std::priority_queue<Record *, vector<Record *>, CustomRecordCompare> pqueue(sortorder);

    for (Page *p : pages) {
        while (p->GetFirst(temp)) {
            pqueue.push(temp);
            temp = new Record();
        }
    }

    pages.clear();

    int currentCount = runlen;

    Page *bufferPage = new Page();
    while (!pqueue.empty()) {
        Record *r = pqueue.top();
        pqueue.pop();
        if (!bufferPage->Append(r)) {
            if (currentCount == 0) {
                pages.push_back(bufferPage);
            } else {
                file->AddPage(bufferPage, writePage++);
                currentCount--;
            }
            bufferPage = new Page();
            bufferPage->Append(r);
        }
    }

    if (currentCount == 0) {
        pages.push_back(bufferPage);
    } else {
        file->AddPage(bufferPage, writePage++);
    }

    return writePage;
}

void Phase2(File *file, Pipe &out, OrderMaker &sortorder, int runlen) {
    int numberOfRuns = std::ceil((file->GetLength() - 1) / (double) runlen);

    SingleRun *runs[numberOfRuns];
    for (int runNumber = 0; runNumber < numberOfRuns; runNumber++)
        runs[runNumber] = new SingleRun(file, runlen, runNumber);

    std::priority_queue<RecordWrapper *, vector<RecordWrapper *>, CustomRecordCompare> pqueue(&sortorder);

    for (int i = 0; i < numberOfRuns; i++) {
        RecordWrapper *recordWrapperTemp = new RecordWrapper(i);
        if (runs[i]->GetFirst(recordWrapperTemp) == 1) pqueue.push(recordWrapperTemp);
    }

    while (!pqueue.empty()) {
        RecordWrapper *r = pqueue.top();
        out.Insert(r->firstOne);
        pqueue.pop();

        RecordWrapper *recordWrapperTemp = new RecordWrapper(r->runArrayIndex);
        if (runs[r->runArrayIndex]->GetFirst(recordWrapperTemp) == 1) pqueue.push(recordWrapperTemp);
    }
}


void Finish(File *file, std::string fileName, Pipe &out) {
    char cstr[fileName.size() + 1];
    strcpy(cstr, fileName.c_str());

    file->Close();
    remove(cstr);
    out.ShutDown();
}

BigQ::~BigQ() {};


SingleRun::SingleRun(File *file, int runLen, int currentRunNumber) {
    this->file = file;
    this->startPage = runLen * currentRunNumber;
    this->endPage = std::min((off_t) (runLen * (currentRunNumber + 1)) - 1, file->GetLength() - 2);
}

SingleRun::~SingleRun() {
    delete page;
}

int SingleRun::GetFirst(RecordWrapper *wrapper) {
    if (page->GetFirst(wrapper->firstOne) != 1) {
        if (startPage > endPage) return 0;

        file->GetPage(page, startPage++);
        return page->GetFirst(wrapper->firstOne);
    }
    return 1;
}