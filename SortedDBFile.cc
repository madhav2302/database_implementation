#include "SortedDBFile.h"
#include <iostream>
#include <fstream>
#include <cstring>

SortedDBFile::SortedDBFile() = default;

SortedDBFile::~SortedDBFile() = default;

int SortedDBFile::Create(const char *fpath, fType file_type, void *startup) {
    this->sortInfo = (SortInfo *) startup;
    return GenericDBFile::Create(fpath, file_type, startup);
}

void SortedDBFile::MoveFirst() {
    GenericDBFile::MoveFirst();

    if (queryInitialized) {
        queryInitialized = false;
        delete query;
    }
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

int SortedDBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
    this->FlushPageIfNeeded();

    // Find common attributes b/w cnf and myOrder and perform binary search to find readCursor
    if (!queryInitialized) {
        query = new OrderMaker();
        cnf.GetSortOrder(*(sortInfo->myOrder), *query);

        readCursor = query->numAtts > 0 ? PerformBinarySearch(cnf, literal) : 0;
        page->EmptyItOut();
        queryInitialized = true;
    }

    int recordAvailable = 0;

    // Fetch record depending on the query
    while ((recordAvailable = page->GetFirst(&fetchme)) == 1 || readCursor < file->GetLength() - 1) {
        if (recordAvailable == 1) {
            if (comp.Compare(&fetchme, &literal, &cnf)) return 1;
            // We don't need to scan records if we have some common query and record in file is greater than the query
            if (query->numAtts > 0 && comp.Compare(&literal, query, &fetchme, sortInfo->myOrder) < 0) {
                page->EmptyItOut();
                readCursor = file->GetLength();
            }
        } else {
            file->GetPage(page, readCursor++);
        }
    }

    return 0;
}

void SortedDBFile::FlushPage() {
    in->ShutDown();

    File tempFile;
    std::string fileName("sorted_" + randomFileName());
    char cstr[fileName.size() + 1];
    strcpy(cstr, fileName.c_str());
    tempFile.Open(0, cstr);

    // Used to write into temp file which contains combined records of existing file and output pipe in sorted order
    Page tempPage;
    off_t tempWritePage = 0;

    // Used for reading record from output pipe
    Record outPipeTempRecord;

    // Used for managing existing file
    int foundRecordInFile = 0;
    Page fileTempPage;
    Record fileTempRecord;
    off_t tempReadCursor = 0;
    off_t fileLength = file->GetLength() - 1;
    bool recordPresentInFileTempRecord = false;

    while (out->Remove(&outPipeTempRecord)) {
        bool outPipeRecordAvailable = true;
        while (recordPresentInFileTempRecord || (foundRecordInFile = fileTempPage.GetFirst(&fileTempRecord)) == 1 ||
               tempReadCursor < fileLength) {
            if (foundRecordInFile) {
                if (comp.Compare(&outPipeTempRecord, &fileTempRecord, sortInfo->myOrder) < 1) {
                    SortedDBFile::AppendRecord(tempFile, tempPage, outPipeTempRecord, tempWritePage);
                    outPipeRecordAvailable = false;
                    recordPresentInFileTempRecord = true;
                    break;
                } else {
                    SortedDBFile::AppendRecord(tempFile, tempPage, fileTempRecord, tempWritePage);
                    recordPresentInFileTempRecord = false;
                }
            } else {
                file->GetPage(&fileTempPage, tempReadCursor++);
            }
        }

        // This will add out pipe record if there are no records remaining in the existing file
        if (outPipeRecordAvailable) SortedDBFile::AppendRecord(tempFile, tempPage, outPipeTempRecord, tempWritePage);
    }

    // Add remaining records from the file
    while ((foundRecordInFile = fileTempPage.GetFirst(&fileTempRecord)) == 1 || tempReadCursor < fileLength) {
        if (foundRecordInFile) SortedDBFile::AppendRecord(tempFile, tempPage, fileTempRecord, tempWritePage);
        else file->GetPage(&fileTempPage, tempReadCursor++);
    }

    tempFile.AddPage(&tempPage, tempWritePage++);
    tempPage.EmptyItOut();

    // Move records to DBFile
    for (int pageNumber = 0; pageNumber < tempFile.GetLength() - 1; pageNumber++) {
        tempFile.GetPage(&tempPage, pageNumber);
        file->AddPage(&tempPage, pageNumber);
    }

    delete bigQ;
    delete in;
    delete out;
    remove(cstr);

    needFlush = false;
}

void SortedDBFile::WriteMetadata(const char *fpath, fType file_type, void *startup) {
    auto *startUpSortInfo = (SortInfo *) startup;
    ofstream metadata(fpath + std::string(".metadata"));
    if (metadata.is_open()) {
        // Write File Type
        metadata << file_type << '\n';

        metadata << startUpSortInfo->runLength << '\n';
        metadata << startUpSortInfo->myOrder->numAtts << '\n';
        for (int index = 0; index < startUpSortInfo->myOrder->numAtts; index++) {
            metadata << startUpSortInfo->myOrder->whichAtts[index] << '\n';
            metadata << startUpSortInfo->myOrder->whichTypes[index] << '\n';
        }
        metadata << '\n';
        metadata.close();
    } else {
        cout << "Unable to open file for write";
        exit(1);
    }
}

void SortedDBFile::ReadMetadata(const char *fpath) {
    auto *orderMaker = new OrderMaker();
    this->sortInfo = new SortInfo(orderMaker, 0);
    string line;
    ifstream metadata_read(fpath + std::string(".metadata"));
    if (metadata_read.is_open()) {
        // Skip file type
        getline(metadata_read, line);

        getline(metadata_read, line);
        this->sortInfo->runLength = std::stoi(line);
        getline(metadata_read, line);

        for (int index = 0; index < std::stoi(line); index++) {
            string att;
            getline(metadata_read, att);

            string type;
            getline(metadata_read, type);
            sortInfo->myOrder->AddOrder(stoi(att), (Type) stoi(type));
        }
        metadata_read.close();
    } else {
        cout << "Unable to open file for read\n";
        exit(1);
    }
}

void SortedDBFile::AppendRecord(File &tempFile, Page &tempPage, Record &addme, off_t &writePage) {
    if (tempPage.Append(&addme) != 1) {
        tempFile.AddPage(&tempPage, writePage++);
        tempPage.EmptyItOut();
        tempPage.Append(&addme);
    }
}

int SortedDBFile::PerformBinarySearch(CNF &cnf, Record &literal) {
    off_t start = 0;
    off_t end = file->GetLength() - 2;

    while (start < end) {
        Page binarySearchPage;
        Record binarySearchRecord;

        off_t mid = (start + end) / 2;

        file->GetPage(&binarySearchPage, mid);
        binarySearchPage.GetFirst(&binarySearchRecord);

        if (comp.Compare(&literal, query, &binarySearchRecord, sortInfo->myOrder) > 0) start = mid + 1;
        else end = mid - 1;
    }

    return start;
}
