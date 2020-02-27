#ifndef DATABASE_IMPLEMENTATION_HEAPDBFILE_H
#define DATABASE_IMPLEMENTATION_HEAPDBFILE_H

#include "GenericDBFile.h"

class HeapDBFile : public GenericDBFile {

private:

    /**
     * Page used for reads and writes.
     */
    Page *page;

    // We keep separate cursor for reading and writing the page, so we don't need to remember any of them.

    /**
     * Current page where we will append the records
     */
    off_t writePage = 0;

    /**
     * Current page from which we will read the data.
     */
    off_t readPage = 0;

    void writeMetadata(const char *fpath, fType file_type, void *startup) override;

    void readMetadata(const char *fpath) override;

    void flushPage() override;

public:
    HeapDBFile();

    ~HeapDBFile() override;

    int Create(const char *fpath, fType file_type, void *startup) override;

    int Open(const char *fpath) override;

    int Close() override;

    void MoveFirst() override;

    void Add(Record &addme) override;

    int GetNext(Record &fetchme) override;

    int GetNext(Record &fetchme, CNF &cnf, Record &literal) override;
};

#endif //DATABASE_IMPLEMENTATION_HEAPDBFILE_H
