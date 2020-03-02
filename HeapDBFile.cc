#include "HeapDBFile.h"
#include <iostream>
#include <fstream>

HeapDBFile::HeapDBFile() = default;

HeapDBFile::~HeapDBFile() = default;;

int HeapDBFile::Create(const char *fpath, fType file_type, void *startup) {
    this->writePage = 0;
    return GenericDBFile::Create(fpath, file_type, startup);
}

int HeapDBFile::Close() {
    this->FlushPageIfNeeded();

    page->EmptyItOut();
    return file->Close();
}

void HeapDBFile::MoveFirst() {
    this->FlushPageIfNeeded();

    this->readCursor = 0;
    page->EmptyItOut();
}

void HeapDBFile::Add(Record &addme) {
    if (page->Append(&addme) == 0) {
        FlushPage();
        page->Append(&addme);
    }

    needFlush = true;
}

int HeapDBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
    this->FlushPageIfNeeded();

    int recordAvailable = 0;

    while ((recordAvailable = page->GetFirst(&fetchme)) == 1 || readCursor < writePage) {
        if (recordAvailable == 1) {
            if (comp.Compare(&fetchme, &literal, &cnf)) return 1;
        } else {
            file->GetPage(page, readCursor++);
        }
    }

    return 0;
}

void HeapDBFile::WriteMetadata(const char *fpath, fType file_type, void *startup) {
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

void HeapDBFile::ReadMetadata(const char *fpath) {
    writePage = file->GetLength() - 1;
}

void HeapDBFile::FlushPage() {
    file->AddPage(page, writePage++);
    page->EmptyItOut();
    needFlush = false;
}
