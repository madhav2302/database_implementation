#include <iostream>
#include "GenericDBFile.h"

GenericDBFile::GenericDBFile() {
    page = new Page();
    file = new File();
    comp = new ComparisonEngine();
}

GenericDBFile::~GenericDBFile() {
    delete page;
    delete file;
    delete comp;
}

int GenericDBFile::Create(const char *fpath, fType file_type, void *startup) {
    this->writeMetadata(fpath, file_type, startup);
    this->file->Open(0, (char *) fpath);
    this->writePage = 0;
    return 1;
}

void GenericDBFile::Load(Schema &f_schema, const char *loadpath) {
    FILE *tableFile = fopen(loadpath, "r");

    Record temp;
    int counter = 0;

    while (temp.SuckNextRecord(&f_schema, tableFile) == 1) {
        counter++;
        Add(temp);

        if (counter % 10000 == 0) std::cerr << counter << "\n";
    }

    // Flush the page with rest of the records
    flushPage();
}

int GenericDBFile::Open(const char *f_path) {
    file->Open(1, (char *) f_path);
    writePage = file->GetLength() - 1;

    return 1;
}

void GenericDBFile::MoveFirst() {
    this->flushPageIfNeeded();

    this->readPage = 0;
    page->EmptyItOut();
    file->GetPage(page, readPage++);
}

int GenericDBFile::Close() {
    this->flushPageIfNeeded();

    page->EmptyItOut();
    return file->Close();
}

void GenericDBFile::Add(Record &rec) {
    if (page->Append(&rec) == 0) {
        flushPage();
        page->Append(&rec);
    }

    needFlush = true;
}

int GenericDBFile::GetNext(Record &fetchme) {
    this->flushPageIfNeeded();

    if (page->GetFirst(&fetchme) == 1) return 1;

    if (readPage < writePage) {
        file->GetPage(page, readPage++);
    }

    return page->GetFirst(&fetchme);
}

void GenericDBFile::flushPageIfNeeded() {
    if (needFlush) {
        cout << "Flushing while switching from writes to read\n";
        flushPage();
    }
}

void GenericDBFile::flushPage() {
    file->AddPage(page, this->writePage++);
    page->EmptyItOut();
    needFlush = false;
}
