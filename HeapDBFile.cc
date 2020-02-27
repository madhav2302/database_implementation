#include "HeapDBFile.h"
#include <iostream>
#include <fstream>

HeapDBFile::HeapDBFile() {
    page = new Page();
    cerr << "Create instance of HeapDBFile\n";
}

HeapDBFile::~HeapDBFile() {
    delete page;
};

int HeapDBFile::Create(const char *fpath, fType file_type, void *startup) {
    this->writePage = 0;
    return GenericDBFile::Create(fpath, file_type, startup);
}

int HeapDBFile::Open(const char *fpath) {
    return GenericDBFile::Open(fpath);
}

int HeapDBFile::Close() {
    this->flushPageIfNeeded();

    page->EmptyItOut();
    return file->Close();
}

void HeapDBFile::MoveFirst() {
    this->flushPageIfNeeded();

    this->readPage = 0;
    page->EmptyItOut();
    file->GetPage(page, readPage++);
}

void HeapDBFile::Add(Record &addme) {
    if (page->Append(&addme) == 0) {
        flushPage();
        page->Append(&addme);
    }

    needFlush = true;
}

int HeapDBFile::GetNext(Record &fetchme) {
    this->flushPageIfNeeded();

    if (page->GetFirst(&fetchme) == 1) return 1;

    if (readPage < writePage) {
        file->GetPage(page, readPage++);
    }

    return page->GetFirst(&fetchme);
}

int HeapDBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
    this->flushPageIfNeeded();

    int foundFilteredValue = 0;

    while ((foundFilteredValue = page->GetFirst(&fetchme)) == 1 || readPage < writePage) {
        if (foundFilteredValue == 1) {
            if (comp->Compare(&fetchme, &literal, &cnf)) {
                return 1;
            }
        } else {
            file->GetPage(page, readPage++);
        }
    }

    return 0;
}

void HeapDBFile::writeMetadata(const char *fpath, fType file_type, void *startup) {
    ofstream metadata(fpath + std::string(".metadata"));
    if (metadata.is_open()) {
        metadata << file_type << '\n';
        metadata << '\n';
        metadata.close();
    } else {
        cout << "Unable to open file for write";
        exit(1);
    }
}

void HeapDBFile::readMetadata(const char *fpath) {
    writePage = file->GetLength() - 1;
}

void HeapDBFile::flushPage() {
    file->AddPage(page, writePage++);
    page->EmptyItOut();
    needFlush = false;
}
