#ifndef REL_OP_H
#define REL_OP_H

#include "Pipe.h"
#include "DBFile.h"
#include "Record.h"
#include "Function.h"

class RelationalOp {

protected:
    pthread_t thread;

    int runLen = 100;
public:
    // blocks the caller until the particular relational operator
    // has run to completion
    void WaitUntilDone() {
        pthread_join(thread, NULL);
    };

    // tell us how much internal memory the operation can use
    void Use_n_Pages(int n) {
        runLen = n;
    }
};

class SelectFile : public RelationalOp {
public:
    void Run(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal);
};

class SelectPipe : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal);
};

class Project : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput);
};

class Join : public RelationalOp {
public:
    void Run(Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal);
};

class DuplicateRemoval : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, Schema &mySchema);
};

class Sum : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, Function &computeMe);
};

class GroupBy : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe);
};

class WriteOut : public RelationalOp {
public:
    void Run(Pipe &inPipe, FILE *outFile, Schema &mySchema);
};

#endif
