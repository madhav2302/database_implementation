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

    /**
     * Page used for reads and writes.
     */
    Page *page;

    /**
     * Current page from which we will read the data.
     */
    off_t readPage = 0;
    off_t writePage = 0;

    SortInfo *sortInfo;

    void writeMetadata(const char *fpath, fType file_type, void *startup) override;

    void readMetadata(const char *fpath) override;

    void flushPage() override;

public:
    SortedDBFile();

    ~SortedDBFile() override;

    int Create(const char *fpath, fType file_type, void *startup) override;

    int Close() override;

    void MoveFirst() override;

    void Add(Record &addme) override;

    int GetNext(Record &fetchme) override;

    int GetNext(Record &fetchme, CNF &cnf, Record &literal) override;
};

#endif //DATABASE_IMPLEMENTATION_SORTEDDBFILE_H
