#ifndef DATABASE_IMPLEMENTATION_HEAPDBFILE_H
#define DATABASE_IMPLEMENTATION_HEAPDBFILE_H

#include "GenericDBFile.h"

class HeapDBFile : public GenericDBFile {

private:

    void writeMetadata(const char *fpath, fType file_type, void *startup) override;
public:
    HeapDBFile();

    ~HeapDBFile() override;

    int Create(const char *fpath, fType file_type, void *startup) override;

    int GetNext(Record &fetchme, CNF &cnf, Record &literal) override;
};

#endif //DATABASE_IMPLEMENTATION_HEAPDBFILE_H
