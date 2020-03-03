#include <iostream>
#include <fstream>
#include "DBFile.h"
#include "HeapDBFile.h"
#include "SortedDBFile.h"

DBFile::DBFile() {}

DBFile::~DBFile() {
}

int DBFile::Create(const char *fpath, fType file_type, void *startup) {
    initInternalFile(file_type);

    return myInternalFile->Create(fpath, file_type, startup);
}

int DBFile::Open(const char *fpath) {
    initInternalFile(readFileType(fpath));

    return myInternalFile->Open(fpath);
}

void DBFile::initInternalFile(fType file_type) {
    if (file_type == heap) {
        this->myInternalFile = new HeapDBFile();
    } else if (file_type == sorted) {
        this->myInternalFile = new SortedDBFile();
    } else {
        cerr << "Unknown file type : " << file_type << '\n';
        exit(1);
    }
}

fType DBFile::readFileType(const char *fpath) {
    ifstream metadata_read(fpath + std::string(".metadata"));
    fType file_type;
    string line;

    if (metadata_read.is_open()) {
        getline(metadata_read, line);
        file_type = (fType) std::stoi(line);
        metadata_read.close();
    } else {
        cout << "Unable to open file for read " << fpath << ".metadata" << '\n';
        exit(1);
    }

    return file_type;
}

int DBFile::Close() {
    int result = myInternalFile->Close();
    delete myInternalFile;
    return result;
}

void DBFile::Load(Schema &myschema, const char *loadpath) {
    myInternalFile->Load(myschema, loadpath);
}

void DBFile::MoveFirst() {
    myInternalFile->MoveFirst();
}

void DBFile::Add(Record &addme) {
    myInternalFile->Add(addme);
}

int DBFile::GetNext(Record &fetchme) {
    return myInternalFile->GetNext(fetchme);
}

int DBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
    return myInternalFile->GetNext(fetchme, cnf, literal);
}






