#ifndef DATABASE_IMPLEMENTATION_RELOPSTRUCTS_H
#define DATABASE_IMPLEMENTATION_RELOPSTRUCTS_H

typedef struct  {
    DBFile *inFile;
    Pipe *outPipe;
    CNF *selOp;
    Record *literal;
} RelOpSelectFileData;

typedef struct {
    Pipe *inPipe;
    Pipe *outPipe;
    CNF *selOp;
    Record *literal;
} RelOpSelectPipeData;

typedef struct {
    Pipe *inPipe;
    Pipe *outPipe;
    int *keepMe;
    int numAttsInput;
    int numAttsOutput;
} RelOpProjectData;

typedef struct {
    Pipe *inPipe;
    Pipe *outPipe;
    Function *computeMe;
    bool distinctFunc;
} RelOpSumData;

typedef struct  {
    Pipe *inPipe;
    Pipe *outPipe;
    Schema *mySchema;
    int runLen;
} RelOpDuplicateRemovalData;

typedef struct  {
    Pipe *inPipe;
    FILE *outFile;
    Schema *mySchema;
} RelOpWriteOutData;

typedef struct {
    Pipe *inPipeL;
    Pipe *inPipeR;
    Pipe *outPipe;
    CNF *selOp;
    Record *literal;
    int runLen;
} RelOpJoinData;

typedef struct {
    Pipe *inPipe;
    Pipe *outPipe;
    OrderMaker *groupAtts;
    Function *computeMe;
    int runLen;
    bool distinctFunc;
} RelOpGroupByData;

#endif //DATABASE_IMPLEMENTATION_RELOPSTRUCTS_H
