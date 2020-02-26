#ifndef DATABASE_IMPLEMENTATION_SORTEDDBFILE_H
#define DATABASE_IMPLEMENTATION_SORTEDDBFILE_H

#include "GenericDBFile.h"
#include "DBFile.h"

class SortedDBFile : public GenericDBFile {

private:
    SortInfo *sortInfo;

    void writeMetadata(const char *fpath, fType file_type, void *startup) override;

    void readMetadata(const char *fpath);

public:
    SortedDBFile();

    ~SortedDBFile() override;

    int Create(const char *fpath, fType file_type, void *startup) override;

    int Open(const char *fpath) override ;

    int Close() override ;

    int GetNext(Record &fetchme, CNF &cnf, Record &literal) override;
};

#endif //DATABASE_IMPLEMENTATION_SORTEDDBFILE_H
