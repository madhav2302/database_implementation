#ifndef DBFILE_H
#define DBFILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Defs.h"
#include "GenericDBFile.h"

class DBFile {

private:
    GenericDBFile *myInternalFile;
public:
    DBFile();

    ~DBFile();

    int Create(const char *fpath, fType file_type, void *startup);

    int Open(const char *fpath);

    int Close();

    void Load(Schema &myschema, const char *loadpath);

    void MoveFirst();

    void Add(Record &addme);

    int GetNext(Record &fetchme);

    int GetNext(Record &fetchme, CNF &cnf, Record &literal);


};

struct SortInfo {
    SortInfo(OrderMaker *myOrder, int runLength) : myOrder(myOrder), runLength(runLength) {
        this->myOrder = myOrder;
        this->runLength = runLength;
    }

    OrderMaker *myOrder;
    int runLength;
};

#endif
