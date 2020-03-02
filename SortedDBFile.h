#ifndef DATABASE_IMPLEMENTATION_SORTEDDBFILE_H
#define DATABASE_IMPLEMENTATION_SORTEDDBFILE_H

#include "GenericDBFile.h"
#include "DBFile.h"
#include "BigQ.h"
#include "Pipe.h"

class SortedDBFile : public GenericDBFile {

private:

    BigQ *bigQ;

    Pipe *in;

    Pipe *out;

    bool queryInitialized = false;

    OrderMaker *query;

    SortInfo *sortInfo;

    void WriteMetadata(const char *fpath, fType file_type, void *startup) override;

    void ReadMetadata(const char *fpath) override;

    void FlushPage() override;

    static void AppendRecord(File &tempFile, Page &tempPage, Record &addme, off_t &writePage);

    int PerformBinarySearch(CNF &cnf, Record &literal);
public:
    SortedDBFile();

    ~SortedDBFile() override;

    int Create(const char *fpath, fType file_type, void *startup) override;

    void MoveFirst() override;

    void Add(Record &addme) override;

    int GetNext(Record &fetchme, CNF &cnf, Record &literal) override;
};

#endif //DATABASE_IMPLEMENTATION_SORTEDDBFILE_H
