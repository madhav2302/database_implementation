#include "SortedDBFile.h"
#include <iostream>
#include <fstream>

SortedDBFile::SortedDBFile() {
    page = new Page();
    cerr << "Create instance of SortedDBFile\n";
}

SortedDBFile::~SortedDBFile() {
    delete page;
}

int SortedDBFile::Create(const char *fpath, fType file_type, void *startup) {
    this->sortInfo = (SortInfo *) startup;
    return GenericDBFile::Create(fpath, file_type, startup);
}

int SortedDBFile::Open(const char *fpath) {
    return GenericDBFile::Open(fpath);
}

int SortedDBFile::Close() {
    this->flushPageIfNeeded();

    return file->Close();
}

void SortedDBFile::MoveFirst() {
    this->flushPageIfNeeded();

    this->readPage = 0;
    page->EmptyItOut();
    file->GetPage(page, readPage++);
}

void SortedDBFile::Add(Record &addme) {
    if (!needFlush) {
        int buffSize = 100;
        in = new Pipe(buffSize);
        out = new Pipe(buffSize);
        bigQ = new BigQ(*(in), *(out), *(sortInfo->myOrder), sortInfo->runLength);
    }

    in->Insert(&addme);
    needFlush = true;
}

int SortedDBFile::GetNext(Record &fetchme) {
    this->flushPageIfNeeded();

    if (page->GetFirst(&fetchme) == 1) return 1;

    if (readPage < file->GetLength() - 1) {
        file->GetPage(page, readPage++);
    }

    return page->GetFirst(&fetchme);
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

void SortedDBFile::flushPage() {
    in->ShutDown();

    File tempFile;
    std::string fileName("sorted_" + randomFileName());
    char cstr[fileName.size() + 1];
    strcpy(cstr, fileName.c_str());
    tempFile.Open(0, cstr);

    Page tempPage;
    Record tempRecord;

    off_t tempWritePage = 0;
    while (out->Remove(&tempRecord)) {
        if (tempPage.Append(&tempRecord) != 1) {
            tempFile.AddPage(&tempPage, tempWritePage++);
            tempPage.EmptyItOut();
            tempPage.Append(&tempRecord);
        }
    }

    tempFile.AddPage(&tempPage, tempWritePage++);
    tempPage.EmptyItOut();
    tempFile.Close();

    // Write into actual file
    tempFile.Open(1, cstr);

    for (int pageNumber = 0; pageNumber < tempFile.GetLength() - 1; pageNumber++) {
        tempFile.GetPage(&tempPage, pageNumber);
        file->AddPage(&tempPage, pageNumber);
    }

    delete bigQ;
    delete in;
    delete out;

    needFlush = false;
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
    writePage = file->GetLength() - 1;

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
