#include <vector>
#include <sstream>
#include "BigQ.h"
#include "File.h"
#include <cmath>
#include <queue>
#include <algorithm>
#include <cstring>

ComparisonEngine comp;
const std::string tablename = "lineitem";

static std::string randomFileName() {
    time_t result = time(nullptr);
    std::stringstream ss;
    ss << result;
    return "tmp" + ss.str() + ".bin";
}

struct CustomCompare {
    bool operator()(RecordWrapper *lhs, RecordWrapper *rhs) {
        return comp.Compare(lhs->firstOne, rhs->firstOne, lhs->sortorder) == 1;
    }
};

BigQ::BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen) {
    std::string fileName(randomFileName());
    char cstr[fileName.size() + 1];
    strcpy(cstr, fileName.c_str());
    file->Open(0, cstr);
    page = new Page();

    Record temp;
    vector<Page *> pages;

    int totalRuns = 0;
    while (in.Remove(&temp)) {
        if (pages.size() == runlen) {
            SortRun(pages, in, out, &sortorder, runlen);
            totalRuns += 1;
        }

        if (!page->Append(&temp)) {
            pages.push_back(page);
            page = new Page();
            page->Append(&temp);
        }
    }

    // Push final page
    pages.push_back(page);

    // Sorting remaining records as run
    SortRun(pages, in, out, &sortorder, runlen);
    totalRuns += 1;

//    cout << "Total runs are  " << totalRuns << '\n';

    int numberOfRuns = std::ceil((file->GetLength() - 1) / (double) runlen);

//    cout << "Number of runs are  " << numberOfRuns << '\n';
    Run *runs[numberOfRuns];

    for (int i = 0; i < numberOfRuns; i++) {
        runs[i] = new Run(file, runlen * i, std::min((off_t) (runlen * (i + 1)) - 1, file->GetLength() - 2));
    }

    std::priority_queue<RecordWrapper *, vector<RecordWrapper *>, CustomCompare> pqueue;

    for (int i = 0; i < numberOfRuns; i++) {
        RecordWrapper *recordWrapperTemp = new RecordWrapper(&sortorder, i);
        if (runs[i]->GetFirst(recordWrapperTemp) == 1) {
            pqueue.push(recordWrapperTemp);
        }
    }

    while (!pqueue.empty()) {
        RecordWrapper *r = pqueue.top();
        out.Insert(r->firstOne);

        RecordWrapper *recordWrapperTemp = new RecordWrapper(&sortorder, r->runArrayIndex);
        if (runs[r->runArrayIndex]->GetFirst(recordWrapperTemp) == 1) {
            pqueue.push(recordWrapperTemp);
        }

        pqueue.pop();
    }

    file->Close();
    remove(cstr);
    out.ShutDown();
}

void BigQ::SortRun(vector<Page *> &pages, Pipe &in, Pipe &out, OrderMaker *sortorder, int runlen) {
    Record temp;
    vector<Record *> records;
    for (Page *p : pages) {
        while (p->GetFirst(&temp)) {
            Record *rec = new Record();
            rec->Consume(&temp);
            records.push_back(rec);
        }
    }

    pages.clear();

    sort(records.begin(), records.end(),
         [sortorder](Record *r1, Record *r2) { return comp.Compare(r1, r2, sortorder) == 1; });

    int currentCount = runlen;

    Page page;
    for (Record *r : records) {
        if (currentCount == 0) {
            in.Insert(r);
        } else if (!page.Append(r)) {
            file->AddPage(&page, writePage++);
            page.EmptyItOut();
            currentCount--;

            if (currentCount == 0) {
                in.Insert(r);
            } else {
                page.Append(r);
            }
        }
    }

    file->AddPage(&page, writePage++);
    page.EmptyItOut();
}

BigQ::~BigQ() {
//    delete file;
}


Run::Run(File *file, int startPage, int endPage) {
    this->file = file;
    this->startPage = startPage;
    this->endPage = endPage;
}

Run::~Run() {
    delete file;
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