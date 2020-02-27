#include <iostream>
#include "GenericDBFile.h"

GenericDBFile::GenericDBFile() {
    file = new File();
    comp = new ComparisonEngine();
}

GenericDBFile::~GenericDBFile() {
    delete file;
    delete comp;
}

int GenericDBFile::Create(const char *fpath, fType file_type, void *startup) {
    this->writeMetadata(fpath, file_type, startup);
    this->file->Open(0, (char *) fpath);
    return 1;
}

int GenericDBFile::Open(const char *f_path) {
    file->Open(1, (char *) f_path);
    readMetadata(f_path);
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

void GenericDBFile::flushPageIfNeeded() {
    if (needFlush) {
        cout << "Flushing while switching from writes to read\n";
        flushPage();
    }
}
