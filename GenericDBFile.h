#ifndef DATABASE_IMPLEMENTATION_GENERICDBFILE_H
#define DATABASE_IMPLEMENTATION_GENERICDBFILE_H


#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Defs.h"

/**
 * Struct used for SortedDBFile which is passed while creating the file
 */
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
     * Current page from which we will read the data.
     */
    off_t readCursor = 0;

    /**
     * The comparator engine is used by GetNext method which filters records based on CNF passed to it.
     */
    ComparisonEngine comp;

    /**
     * Used as a flag to determine if page need to be flushed while switching from writes to read.
     */
    bool needFlush = false;

    /**
     * Flush the page if it have some records while switching from writes to read.
     */
    void FlushPageIfNeeded();

    /**
     * Flush the page into the file.
     * The page won't have any records after it.
     */
    virtual void FlushPage() = 0;

    /**
     * Writes information in metadata file related to different version of DBFiles
     */
    virtual void WriteMetadata(const char *fpath, fType file_type, void *startup) = 0;

    /**
     * Read information for the DBFile
     */
    virtual void ReadMetadata(const char *fpath) = 0;

public:
    GenericDBFile();

    virtual ~GenericDBFile();

    virtual int Create(const char *fpath, fType file_type, void *startup);

    virtual void MoveFirst();

    virtual void Add(Record &addme) = 0;

    virtual int GetNext(Record &fetchme, CNF &cnf, Record &literal) = 0;

    int Open(const char *fpath);

    int Close();

    void Load(Schema &myschema, const char *loadpath);

    int GetNext(Record &fetchme);
};


#endif //DATABASE_IMPLEMENTATION_GENERICDBFILE_H
