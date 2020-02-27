#ifndef DATABASE_IMPLEMENTATION_GENERICDBFILE_H
#define DATABASE_IMPLEMENTATION_GENERICDBFILE_H


#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Defs.h"

struct SortInfo {
    SortInfo(OrderMaker *myOrder, int runLength) : myOrder(myOrder), runLength(runLength) {
        this->myOrder = myOrder;
        this->runLength = runLength;
    }

    OrderMaker *myOrder;
    int runLength;
};

class GenericDBFile {
protected:
    /**
     * Object of a file.
     */
    File *file;

    /**
     * The comparator engine is used by GetNext method which filters records based on CNF passed to it.
     */
    ComparisonEngine *comp;

    /**
     * Used as a flag to determine if page need to be flushed while switching from writes to read.
     */
    bool needFlush = false;

    /**
     * Flush the page if it have some records while switching from writes to read.
     */
    void flushPageIfNeeded();

    /**
     * Flush the page into the file.
     * The page won't have any records after it.
     */
    virtual void flushPage() = 0;

    virtual void writeMetadata(const char *fpath, fType file_type, void *startup) = 0;

    virtual void readMetadata(const char *fpath) = 0;

public:
    GenericDBFile();

    virtual ~GenericDBFile();

    virtual int Create(const char *fpath, fType file_type, void *startup);

    int Open(const char *fpath);

    virtual int Close() = 0;

    void Load(Schema &myschema, const char *loadpath);

    virtual void MoveFirst() = 0;

    virtual void Add(Record &addme) = 0;

    virtual int GetNext(Record &fetchme) = 0;

    virtual int GetNext(Record &fetchme, CNF &cnf, Record &literal) = 0;
};


#endif //DATABASE_IMPLEMENTATION_GENERICDBFILE_H
