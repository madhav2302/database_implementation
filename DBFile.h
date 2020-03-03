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
    /**
     * Instance of HeapDBFile or SortedDBFile depending on the file type passed with the file name
     */
    GenericDBFile *myInternalFile;

    /**
     * Reads file type saved in metadata file when opening file
     */
    static fType readFileType(const char *fpath);

    /**
     * Initialises myInternalFile variable depending on the file type
     */
    void initInternalFile(fType f_type);
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
