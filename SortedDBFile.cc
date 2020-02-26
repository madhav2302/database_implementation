#include "SortedDBFile.h"
#include <iostream>
#include <fstream>

SortedDBFile::SortedDBFile() {
    cerr << "Create instance of SortedDBFile\n";
}

SortedDBFile::~SortedDBFile() {
}

int SortedDBFile::Create(const char *fpath, fType file_type, void *startup) {
    this->sortInfo = (SortInfo *) startup;
    return GenericDBFile::Create(fpath, file_type, startup);
}

int SortedDBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
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

void SortedDBFile::writeMetadata(const char *fpath, fType file_type, void *startup) {
    SortInfo *startUpSortInfo = (SortInfo *) startup;
    ofstream metadata(fpath + std::string(".metadata"));
    if (metadata.is_open()) {
        // Write File Type
        metadata << file_type << '\n';

        metadata << startUpSortInfo->runLength << '\n';
        metadata << startUpSortInfo->myOrder->getNumAtts() << '\n';
        for (int index = 0; index < startUpSortInfo->myOrder->getNumAtts(); index++) {
            metadata << startUpSortInfo->myOrder->getWhichAtts(index) << '\n';
            metadata << startUpSortInfo->myOrder->getWhichTypes(index) << '\n';
        }
        metadata << '\n';
        metadata.close();
    } else {
        cout << "Unable to open file for write";
        exit(1);
    }
}

void SortedDBFile::readMetadata(const char *fpath) {
    OrderMaker *orderMaker = new OrderMaker();
    this->sortInfo = new SortInfo(orderMaker, 0);
    string line;
    ifstream metadata_read(fpath + std::string(".metadata"));
    if (metadata_read.is_open()) {
        // Skip file type
        getline(metadata_read, line);

        getline(metadata_read, line);
        this->sortInfo->runLength = std::stoi(line);
        getline(metadata_read, line);
        int numAtts = std::stoi(line);

        this->sortInfo->myOrder->setNumAtts(numAtts);

        for (int index = 0; index < numAtts; index++) {
            getline(metadata_read, line);
            this->sortInfo->myOrder->setWhichAtts(index, stoi(line));

            getline(metadata_read, line);
            this->sortInfo->myOrder->setWhichTypes(index, (Type) stoi(line));
        }
        metadata_read.close();
    } else {
        cout << "Unable to open file for read\n";
        exit(1);
    }
}

int SortedDBFile::Open(const char *fpath) {
    readMetadata(fpath);
    return GenericDBFile::Open(fpath);
}

int SortedDBFile::Close() {
    return GenericDBFile::Close();
}


