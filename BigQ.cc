#include <vector>
#include <cmath>
#include <queue>
#include <algorithm>
#include <cstring>
#include "BigQ.h"
#include "File.h"

ComparisonEngine comp;

BigQ::BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen) {
    std::string fileName(randomFileName());
    char cstr[fileName.size() + 1];
    strcpy(cstr, fileName.c_str());
    file->Open(0, cstr);
    page = new Page();

    auto *temp = new Record();
    vector<Page *> pages;

    while (in.Remove(temp)) {
        if (pages.size() == runlen) SortRun(pages, out, &sortorder, runlen);

        if (!page->Append(temp)) {
            pages.push_back(page);
            page = new Page();
            page->Append(temp);
        }

        temp = new Record();
    }

    // Push final page
    pages.push_back(page);

    // Sorting remaining records as run
    while (!pages.empty()) SortRun(pages, out, &sortorder, runlen);

    int numberOfRuns = std::ceil((file->GetLength() - 1) / (double) runlen);

    Run *runs[numberOfRuns];

    for (int i = 0; i < numberOfRuns; i++) {
        runs[i] = new Run(file, runlen * i, std::min((off_t) (runlen * (i + 1)) - 1, file->GetLength() - 2));
    }

    std::priority_queue<RecordWrapper *, vector<RecordWrapper *>, CustomRecordCompare> pqueue(&sortorder);

    for (int i = 0; i < numberOfRuns; i++) {
        auto *recordWrapperTemp = new RecordWrapper(i);
        if (runs[i]->GetFirst(recordWrapperTemp) == 1) {
            pqueue.push(recordWrapperTemp);
        }
    }

    while (!pqueue.empty()) {
        RecordWrapper *r = pqueue.top();
        out.Insert(r->firstOne);
        pqueue.pop();

        auto *recordWrapperTemp = new RecordWrapper(r->runArrayIndex);
        if (runs[r->runArrayIndex]->GetFirst(recordWrapperTemp) == 1) {
            pqueue.push(recordWrapperTemp);
        }
    }

    file->Close();
    remove(cstr);
    out.ShutDown();
}

void BigQ::SortRun(vector<Page *> &pages, Pipe &out, OrderMaker *sortorder, int runlen) {
    auto *temp = new Record();

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
}

BigQ::~BigQ() {
    delete file;
    delete page;
}


Run::Run(File *file, int startPage, int endPage) {
    this->file = file;
    this->startPage = startPage;
    this->endPage = endPage;
}

Run::~Run() {
    delete page;
}

int Run::GetFirst(RecordWrapper *wrapper) {
    if (page->GetFirst(wrapper->firstOne) != 1) {
        if (startPage > endPage) return 0;

        file->GetPage(page, startPage++);
        return page->GetFirst(wrapper->firstOne);
    }
    return 1;
}