#include <iostream>
#include "GenericDBFile.h"

GenericDBFile::GenericDBFile() {
    file = new File();
    page = new Page();
}

GenericDBFile::~GenericDBFile() {
    delete page;
    delete file;
}

int GenericDBFile::Create(const char *fpath, fType file_type, void *startup) {
    this->WriteMetadata(fpath, file_type, startup);
    this->file->Open(0, (char *) fpath);
    return 1;
}

int GenericDBFile::Open(const char *f_path) {
    file->Open(1, (char *) f_path);
    ReadMetadata(f_path);
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
    FlushPage();
    MoveFirst();
}

void GenericDBFile::MoveFirst() {
    this->FlushPageIfNeeded();

    this->readCursor = 0;
    page->EmptyItOut();
}

int GenericDBFile::Close() {
    this->FlushPageIfNeeded();

    page->EmptyItOut();
    return file->Close();
}

int GenericDBFile::GetNext(Record &fetchme) {
    this->FlushPageIfNeeded();

    if (page->GetFirst(&fetchme) == 1) return 1;
    if (readCursor < file->GetLength() - 1) file->GetPage(page, readCursor++);
    return page->GetFirst(&fetchme);
}

void GenericDBFile::FlushPageIfNeeded() {
    if (!needFlush) return;

    cout << "Flushing while switching from writes to read\n";
    FlushPage();
    MoveFirst();
}
