#ifndef DATABASE_IMPLEMENTATION_RELOPSTRUCTS_H
#define DATABASE_IMPLEMENTATION_RELOPSTRUCTS_H

typedef struct  {
    DBFile *inFile;
    Pipe *outPipe;
    CNF *selOp;
    Record *literal;
} RelOpSelectFileData;

void *SelectFileFunction(void *data);

typedef struct {
    Pipe *inPipe;
    Pipe *outPipe;
    CNF *selOp;
    Record *literal;
} RelOpSelectPipeData;

void *SelectPipeFunction(void *data);

typedef struct {
    Pipe *inPipe;
    Pipe *outPipe;
    int *keepMe;
    int numAttsInput;
    int numAttsOutput;
} RelOpProjectData;

void *ProjectFunction(void *data);

typedef struct {
    Pipe *inPipe;
    Pipe *outPipe;
    Function *computeMe;
} RelOpSumData;

void *SumFunction(void *data);

typedef struct  {
    Pipe *inPipe;
    Pipe *outPipe;
    Schema *mySchema;
    int runLen;
} RelOpDuplicateRemovalData;

void *DuplicateRemoveFunction(void *data);


typedef struct  {
    Pipe *inPipe;
    FILE *outFile;
    Schema *mySchema;
} RelOpWriteOutData;

void *WriteOutFunction(void *data);

typedef struct {
    Pipe *inPipeL;
    Pipe *inPipeR;
    Pipe *outPipe;
    CNF *selOp;
    Record *literal;
    int runLen;
} RelOpJoinData;

void *JoinFunction(void *data);

typedef struct {
    Pipe *inPipe;
    Pipe *outPipe;
    OrderMaker *groupAtts;
    Function *computeMe;
    int runLen;
} RelOpGroupByData;

void *GroupByFunction(void *data);

#endif //DATABASE_IMPLEMENTATION_RELOPSTRUCTS_H
