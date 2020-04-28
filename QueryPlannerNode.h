#ifndef DATABASE_IMPLEMENTATION_QUERY_PLAN__NODE_H
#define DATABASE_IMPLEMENTATION_QUERY_PLAN__NODE_H

#include "Schema.h"
#include "Function.h"
#include "Comparison.h"
#include "RelOp.h"
#include <unordered_map>

class RelOpNode {
public:
    RelOpNode *child1;
    int inputPipeId1 = -1;

    RelOpNode *child2;
    int inputPipeId2 = -1;

    Schema *outputSchema;
    int outputPipeId = -1;

    Pipe *pipe = new Pipe(100);

    virtual void Print();

    virtual void Execute(unordered_map<int, Pipe *> *pipes) = 0;

    virtual void CleanUp() = 0;
};


class DuplicateRemovalRelOpNode : public RelOpNode {
public:
    Schema *inputSchema;

    DuplicateRemoval *relOp = new DuplicateRemoval();

    void Print() override;

    void Execute(unordered_map<int, Pipe *> *pipes) override;

    void CleanUp() override;
};


class GroupByRelOpNode : public RelOpNode {
public:
    OrderMaker *groupAtts;
    Function *computeMe;
    int distinctFunc;

    GroupBy *relOp = new GroupBy();

    void Print() override;

    void Execute(unordered_map<int, Pipe *> *pipes) override;

    void CleanUp() override;
};


class JoinRelOpNode : public RelOpNode {
public:
    CNF *selOp;
    Record *literal;

    Join *relOp = new Join();

    void Print() override;

    void Execute(unordered_map<int, Pipe *> *pipes) override;

    void CleanUp() override;
};


class SelectFileRelOpNode : public RelOpNode {
public:
    CNF *selOp;
    Record *literal = nullptr;

    DBFile *dbFile = new DBFile();
    SelectFile *relOp = new SelectFile();

    void Print() override;

    void Execute(unordered_map<int, Pipe *> *pipes) override;

    void CleanUp() override;
};


class SelectPipeRelOpNode : public RelOpNode {
public:
public:
    CNF *selOp = nullptr;
    Record *literal = nullptr;

    SelectPipe *relOp = new SelectPipe();

    void Print() override;

    void Execute(unordered_map<int, Pipe *> *pipes) override;

    void CleanUp() override;
};

class SumRelOpNode : public RelOpNode {
public:
    Function *computeMe;
    int distinctFunc;

    Sum *relOp = new Sum();

    void Print() override;

    void Execute(unordered_map<int, Pipe *> *pipes) override;

    void CleanUp() override;
};

class ProjectRelOpNode : public RelOpNode {
public:
    int *keepMe;
    int numAttsInput;
    int numAttsOutput;

    Project *relOp = new Project();

    void Print() override;

    void Execute(unordered_map<int, Pipe *> *pipes) override;

    void CleanUp() override;
};

#endif 