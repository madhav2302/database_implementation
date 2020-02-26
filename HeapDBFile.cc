#include "HeapDBFile.h"
#include <iostream>
#include <fstream>

HeapDBFile::HeapDBFile() {
    cerr << "Create instance of HeapDBFile\n";
}

HeapDBFile::~HeapDBFile() = default;

int HeapDBFile::Create(const char *fpath, fType file_type, void *startup) {
    return GenericDBFile::Create(fpath, file_type, startup);
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




