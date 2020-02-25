#include "DBFile.h"

DBFile::DBFile() {
    myInternalFile = new GenericDBFile();
}

DBFile::~DBFile() {
    delete myInternalFile;
}

int DBFile::Create(const char *fpath, fType file_type, void *startup) {
    return myInternalFile->Create(fpath, file_type, startup);
}

int DBFile::Open(const char *fpath) {
    return myInternalFile->Open(fpath);
}

int DBFile::Close() {
    return myInternalFile->Close();
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






