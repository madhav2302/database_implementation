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
    void WaitUntilDone();

    // tell us how much internal memory the operation can use
    void Use_n_Pages(int n);
};

class SelectFile : public RelationalOp {
public:
    void Run(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal);

    static void *ThreadMethod(void *d);
};

class SelectPipe : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal);

    static void *ThreadMethod(void *data);
};

class Project : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput);

    static void *ThreadMethod(void *data);
};

class Join : public RelationalOp {
public:
    void Run(Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal);

    static void writeToFile(std::string fileName, Pipe *pipe);

    static void NestedBlockJoin(Pipe *pipeL, Pipe *pipeR, int runlen, Pipe *out);

    static void ComparisonBasedJoin(Pipe *pipeL, Pipe *pipeR, OrderMaker *orderL, OrderMaker *orderR, Pipe *out);

    static void *ThreadMethod(void *data);
};

class DuplicateRemoval : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, Schema &mySchema);

    static void *ThreadMethod(void *data);
};

class Sum : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, Function &computeMe);

    static void *ThreadMethod(void *data);
};

class GroupBy : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe);

    static void *ThreadMethod(void *data);
};

class WriteOut : public RelationalOp {
public:
    void Run(Pipe &inPipe, FILE *outFile, Schema &mySchema);

    static void *ThreadMethod(void *data);
};

#endif
