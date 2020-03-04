#ifndef DATABASE_IMPLEMENTATION_RELOPSTRUCTS_H
#define DATABASE_IMPLEMENTATION_RELOPSTRUCTS_H

struct RelOpSelectFileData {
    RelOpSelectFileData(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal) : inFile(inFile), outPipe(outPipe),
                                                                                      selOp(selOp),
                                                                                      literal(literal) {
        this->inFile = inFile;
        this->outPipe = outPipe;
        this->selOp = selOp;
        this->literal = literal;
    }

    DBFile &inFile;
    Pipe &outPipe;
    CNF &selOp;
    Record &literal;
};

void *SelectFileFunction(void *data);

struct RelOpSelectPipeData {
    RelOpSelectPipeData(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal) : inPipe(inPipe), outPipe(outPipe),
                                                                                    selOp(selOp),
                                                                                    literal(literal) {
        this->inPipe = inPipe;
        this->outPipe = outPipe;
        this->selOp = selOp;
        this->literal = literal;
    }

    Pipe &inPipe;
    Pipe &outPipe;
    CNF &selOp;
    Record &literal;
};

void *SelectPipeFunction(void *data);

struct RelOpProjectData {
    RelOpProjectData(Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput) : inPipe(inPipe),
                                                                                                      outPipe(outPipe),
                                                                                                      keepMe(keepMe),
                                                                                                      numAttsInput(
                                                                                                              numAttsInput),
                                                                                                      numAttsOutput(
                                                                                                              numAttsOutput) {
        this->inPipe = inPipe;
        this->outPipe = outPipe;
        this->keepMe = keepMe;
        this->numAttsInput = numAttsInput;
        this->numAttsOutput = numAttsOutput;
    }

    Pipe &inPipe;
    Pipe &outPipe;
    int *keepMe;
    int numAttsInput;
    int numAttsOutput;
};

void *ProjectFunction(void *data);

struct RelOpSumData {
    RelOpSumData(Pipe &inPipe, Pipe &outPipe, Function &computeMe) : inPipe(inPipe),
                                                                     outPipe(outPipe),
                                                                     computeMe(computeMe) {
        this->inPipe = inPipe;
        this->outPipe = outPipe;
        this->computeMe = computeMe;
    }

    Pipe &inPipe;
    Pipe &outPipe;
    Function &computeMe;
};

void *SumFunction(void *data);

struct RelOpDuplicateRemovalData {
    RelOpDuplicateRemovalData(Pipe &inPipe, Pipe &outPipe, Schema &mySchema, int runLen) : inPipe(inPipe),
                                                                                           outPipe(outPipe),
                                                                                           mySchema(mySchema),
                                                                                           runLen(runLen) {
        this->inPipe = inPipe;
        this->outPipe = outPipe;
        this->mySchema = mySchema;
    }

    Pipe &inPipe;
    Pipe &outPipe;
    Schema &mySchema;
    int runLen;
};

void *DuplicateRemoveFunction(void *data);


struct RelOpWriteOutData {
    RelOpWriteOutData(Pipe &inPipe, FILE *outFile, Schema &mySchema) : inPipe(inPipe),
                                                                       outFile(outFile),
                                                                       mySchema(mySchema) {
        this->inPipe = inPipe;
        this->outFile = outFile;
        this->mySchema = mySchema;
    }

    Pipe &inPipe;
    FILE *outFile;
    Schema &mySchema;
};

void *WriteOutFunction(void *data);
#endif //DATABASE_IMPLEMENTATION_RELOPSTRUCTS_H
