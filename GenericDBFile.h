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
     * Page used for reads and writes.
     */
    Page *page;

    /**
     * The comparator engine is used by GetNext method which filters records based on CNF passed to it.
     */
    ComparisonEngine *comp;

    // We keep separate cursor for reading and writing the page, so we don't need to remember any of them.

    /**
     * Current page where we will append the records
     */
    off_t writePage = 0;

    /**
     * Current page from which we will read the data.
     */
    off_t readPage = 0;

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
    void flushPage();

    virtual void writeMetadata(const char *fpath, fType file_type, void *startup) = 0;

public:
    GenericDBFile();

    virtual ~GenericDBFile();

    virtual int Create(const char *fpath, fType file_type, void *startup);

    virtual int Open(const char *fpath);

    virtual int Close();

    void Load(Schema &myschema, const char *loadpath);

    void MoveFirst();

    void Add(Record &addme);

    int GetNext(Record &fetchme);

    virtual int GetNext(Record &fetchme, CNF &cnf, Record &literal) = 0;
};


#endif //DATABASE_IMPLEMENTATION_GENERICDBFILE_H
