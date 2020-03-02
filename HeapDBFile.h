#ifndef DATABASE_IMPLEMENTATION_HEAPDBFILE_H
#define DATABASE_IMPLEMENTATION_HEAPDBFILE_H

#include "GenericDBFile.h"

class HeapDBFile : public GenericDBFile {

private:

    /**
     * Current page where we will append the records
     */
    off_t writePage = 0;

    void WriteMetadata(const char *fpath, fType file_type, void *startup) override;

    void ReadMetadata(const char *fpath) override;

    void FlushPage() override;

public:
    HeapDBFile();

    ~HeapDBFile() override;

    int Create(const char *fpath, fType file_type, void *startup) override;

    int Close() override;

    void MoveFirst() override;

    void Add(Record &addme) override;

    int GetNext(Record &fetchme, CNF &cnf, Record &literal) override;
};

#endif //DATABASE_IMPLEMENTATION_HEAPDBFILE_H
