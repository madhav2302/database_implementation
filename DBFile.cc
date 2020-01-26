#include <iostream>
#include "Record.h"
#include "Schema.h"
#include "Comparison.h"
#include "DBFile.h"

DBFile::DBFile() {
    page = new Page();
    file = new File();
    comp = new ComparisonEngine();
}

DBFile::~DBFile() {
    delete page;
    delete file;
    delete comp;
}

int DBFile::Create(const char *f_path, fType f_type, void *startup) {
    this->fileType = f_type;
    this->file->Open(0, (char *) f_path);
    this->writePage = 0;
    return 1;
}

void DBFile::Load(Schema &f_schema, const char *loadpath) {
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

int DBFile::Open(const char *f_path) {
    file->Open(1, (char *) f_path);
    writePage = file->GetLength() - 1;

    return 1;
}

void DBFile::MoveFirst() {
    this->flushPageIfNeeded();

    this->readPage = 0;
    page->EmptyItOut();
    file->GetPage(page, readPage++);
}

int DBFile::Close() {
    this->flushPageIfNeeded();

    page->EmptyItOut();
    return file->Close();
}

void DBFile::Add(Record &rec) {
    if (page->Append(&rec) == 0) {
        flushPage();
        page->Append(&rec);
    }

    needFlush = true;
}

int DBFile::GetNext(Record &fetchme) {
    this->flushPageIfNeeded();

    if (page->GetFirst(&fetchme) == 1) return 1;

    if (readPage < writePage) {
        file->GetPage(page, readPage++);
    }

    return page->GetFirst(&fetchme);
}


int DBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
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

void DBFile::flushPageIfNeeded() {
    if (needFlush) {
        cout << "Flushing while switching from writes to read\n";
        flushPage();
    }
}

void DBFile::flushPage() {
    file->AddPage(page, this->writePage++);
    page->EmptyItOut();
    needFlush = false;
}
