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

    virtual string operation() = 0;
};

class SelectFile : public RelationalOp {
public:
    void Run(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal);

    static void *ThreadMethod(void *d);

    string operation() override;
};

class SelectPipe : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal);

    static void *ThreadMethod(void *data);

    string operation() override;
};

class Project : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput);

    static void *ThreadMethod(void *data);

    string operation() override;
};

class Join : public RelationalOp {
public:
    void Run(Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal);

    static void writeToFile(std::string fileName, Pipe *pipe);

    static void NestedBlockJoin(Pipe *pipeL, Pipe *pipeR, int runlen, Pipe *out);

    static void ComparisonBasedJoin(Pipe *pipeL, Pipe *pipeR, OrderMaker *orderL, OrderMaker *orderR, Pipe *out);

    static void *ThreadMethod(void *data);

    string operation() override;
};

class DuplicateRemoval : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, Schema &mySchema);

    static void *ThreadMethod(void *data);

    string operation() override;
};

class Sum : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, Function &computeMe, bool distinctFunc);

    static void *ThreadMethod(void *data);

    string operation() override;
};

class GroupBy : public RelationalOp {
public:
    void Run(Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe, bool distinctFunc);

    static void *ThreadMethod(void *data);

    string operation() override;
};

class WriteOut : public RelationalOp {
public:
    void Run(Pipe &inPipe, FILE *outFile, Schema &mySchema);

    static void *ThreadMethod(void *data);

    string operation() override;
};

#endif
