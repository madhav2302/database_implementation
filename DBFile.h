#ifndef DBFILE_H
#define DBFILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"

typedef enum {
    heap, sorted, tree
} fType;

// stub DBFile header..replace it with your own DBFile.h 

class DBFile {

private:

    /**
     * Stores the file type of DBFile
     */
    fType fileType;

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

#endif
