#ifndef DATABASE_IMPLEMENTATION_SORTEDDBFILE_H
#define DATABASE_IMPLEMENTATION_SORTEDDBFILE_H

#include "GenericDBFile.h"
#include "DBFile.h"
#include "BigQ.h"
#include "Pipe.h"

class SortedDBFile : public GenericDBFile {

private:

    /**
     * Used to sort records added into the DBFile
     */
    BigQ *bigQ;

    /**
     * Insert records into in, whenever a new record is added
     */
    Pipe *in;

    /**
     * Fetches sorted records through out
     */
    Pipe *out;

    /**
     * No need to initialize query again, as we consider same CNF will be passed in consecutive GetNext calls
     */
    bool queryInitialized = false;

    /**
     * We construct query from CNF and SortOrder of file which is used for binary search b/w pages
     */
    OrderMaker *query;

    /**
     * The information related to OrderMaker and run length passed in Create method
     */
    SortInfo *sortInfo;

    void WriteMetadata(const char *fpath, fType file_type, void *startup) override;

    void ReadMetadata(const char *fpath) override;

    void FlushPage() override;

    /**
     * Appends record into the page
     */
    static void AppendRecord(File &tempFile, Page &tempPage, Record &addme, off_t &writePage);

    /**
     * Perform binary search upon query to find the page to start reading
     */
    int PerformBinarySearch(CNF &cnf, Record &literal);
public:
    SortedDBFile();

    ~SortedDBFile() override;

    int Create(const char *fpath, fType file_type, void *startup) override;

    void MoveFirst() override;

    void Add(Record &addme) override;

    int GetNext(Record &fetchme, CNF &cnf, Record &literal) override;
};

#endif //DATABASE_IMPLEMENTATION_SORTEDDBFILE_H
