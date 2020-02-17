#include <vector>
#include <sstream>
#include "BigQ.h"
#include "File.h"

ComparisonEngine comp;
const std::string tablename = "lineitem";

static std::string randomFileName() {
    std::time_t result = std::time(nullptr);
    std::stringstream ss;
    ss << result;
    return "tmp" + ss.str() + ".bin";
}

BigQ::BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen) {
    std::string fileName(randomFileName());
    char cstr[fileName.size() + 1];
    std::strcpy(cstr, fileName.c_str());
    file->Open(0, cstr);
    page = new Page();

    Record temp;
    vector<Page *> pages;

    while (in.Remove(&temp)) {
        if (pages.size() == runlen) SortRun(pages, in, out, &sortorder, runlen);

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

    for (int whichPage = 0; whichPage < file->GetLength() - 1; whichPage++) {
        int numberOfRecords = 0;
        file->GetPage(page, whichPage);
        while (page->GetFirst(&temp) == 1) {
            numberOfRecords++;

            out.Insert(&temp);
        }
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

    std::sort(records.begin(), records.end(),
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