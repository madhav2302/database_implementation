#ifndef DATABASE_IMPLEMENTATION_QUERY_PLAN__NODE_H
#define DATABASE_IMPLEMENTATION_QUERY_PLAN__NODE_H

#include "Schema.h"
#include "Function.h"
#include "Comparison.h"

class RelOpNode {
public:
    RelOpNode *child1;
    int inputPipeId1 = -1;

    RelOpNode *child2;
    int inputPipeId2 = -1;

    Schema *outputSchema;
    int outputPipeId = -1;

    virtual void Print();
};


class DuplicateRemovalRelOpNode : public RelOpNode {
public:
    Schema *inputSchema;

    void Print() override;
};


class GroupByRelOpNode : public RelOpNode {
public:
    OrderMaker *groupAtts;
    Function *computeMe;
    int distinctFunc;

    void Print() override;
};


class JoinRelOpNode : public RelOpNode {
public:
    CNF *selOp;
    Record *literal;

    void Print() override;
};


class SelectFileRelOpNode : public RelOpNode {
public:
    CNF *selOp;
    Record *literal;

    void Print() override;
};


class SelectPipeRelOpNode : public RelOpNode {
public:
public:
    CNF *selOp;
    Record *literal;

    void Print() override;
};

class SumRelOpNode : public RelOpNode {
public:
    Function *computeMe;
    int distinctFunc;

    void Print() override;
};

class ProjectRelOpNode : public RelOpNode {
public:
    int *keepMe;
    int numAttsInput;
    int numAttsOutput;

    void Print() override;
};

#endif 