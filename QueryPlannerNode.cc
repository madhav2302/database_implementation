
#include "QueryPlannerNode.h"
#include <iostream>
#include "Pipe.h"

using namespace std;

void RelOpNode::Print() {
    if (inputPipeId1 != -1) cout << "Input Pipe " << inputPipeId1 << "\n";
    if (inputPipeId2 != -1) cout << "Input Pipe " << inputPipeId2 << "\n";
    if (outputPipeId != -1) cout << "Output Pipe " << outputPipeId << "\n";

    if (outputSchema) {
        cout << "Output Schema:" << "\n";
        outputSchema->Print();
    }
}

void DuplicateRemovalRelOpNode::Print() {
    cout << " *********** " << "\n";
    cout << "DISTINCT operation" << "\n";

    RelOpNode::Print();
}

void DuplicateRemovalRelOpNode::Execute(unordered_map<int, Pipe *> *pipes) {
    pipes->insert(pair<int, Pipe *>(outputPipeId, pipe));

    Pipe *inputPipe = pipes->find(inputPipeId1)->second;
    relOp->Run(*inputPipe, *pipe, *outputSchema);
}

void DuplicateRemovalRelOpNode::CleanUp() {
    relOp->WaitUntilDone();
}

void GroupByRelOpNode::Print() {
    cout << " *********** " << "\n";
    cout << "GROUP BY operation" << "\n";

    RelOpNode::Print();
    cout << "\n";

    if (groupAtts) {
        cout << "GROUPING ON" << "\n";
        groupAtts->Print();
        cout << "\n";
    }

    if (computeMe) {
        cout << "FUNCTION" << "\n";
        computeMe->Print(child1->outputSchema);
        cout << "\n";
        cout << "Distinct Function: " << distinctFunc << "\n";
        cout << "\n";
    }
}

void GroupByRelOpNode::Execute(unordered_map<int, Pipe *> *pipes) {
    pipes->insert(pair<int, Pipe *>(outputPipeId, pipe));

    Pipe *inputPipe = pipes->find(inputPipeId1)->second;
    relOp->Run(*inputPipe, *pipe, *groupAtts, *computeMe, distinctFunc);
}

void GroupByRelOpNode::CleanUp() {
    relOp->WaitUntilDone();
}

void JoinRelOpNode::Print() {
    cout << " *********** " << "\n";
    cout << "JOIN operation" << "\n";

    RelOpNode::Print();
    cout << "\n";

    if (!selOp) return;

    cout << "CNF: " << "\n";
    selOp->Print();
    cout << "\n";
}

void JoinRelOpNode::Execute(unordered_map<int, Pipe *> *pipes) {
    pipes->insert(pair<int, Pipe *>(outputPipeId, pipe));

    Pipe *leftPipe = pipes->find(inputPipeId1)->second;
    Pipe *rightPipe = pipes->find(inputPipeId2)->second;
    relOp->Run(*leftPipe, *rightPipe, *pipe, *selOp, *literal);
}

void JoinRelOpNode::CleanUp() {
    relOp->WaitUntilDone();
}

void SelectFileRelOpNode::Print() {
    cout << " *********** " << "\n";
    cout << "SELECT FILE operation" << "\n";

    RelOpNode::Print();
    cout << "\n";

    if (!selOp) return;

    cout << "SELECTION CNF :" << "\n";
    selOp->Print();
    cout << "\n";
}

void SelectFileRelOpNode::Execute(unordered_map<int, Pipe *> *pipes) {
    pipes->insert(pair<int, Pipe *>(outputPipeId, pipe));
    dbFile->Open(outputSchema->binaryFileLocation());
    dbFile->MoveFirst();
    relOp->Run(*dbFile, *pipe, *selOp, *literal);
}

void SelectFileRelOpNode::CleanUp() {
    relOp->WaitUntilDone();
    dbFile->Close();
}

void SelectPipeRelOpNode::Print() {
    cout << " *********** " << "\n";
    cout << "SELECT PIPE operation" << "\n";
    RelOpNode::Print();
    cout << "\n";

    if (!selOp) return;

    cout << "SELECTION CNF :" << "\n";
    selOp->Print();
    cout << "\n";
}

void SelectPipeRelOpNode::Execute(unordered_map<int, Pipe *> *pipes) {
    pipes->insert(pair<int, Pipe *>(outputPipeId, pipe));

    Pipe *inputPipe = pipes->find(inputPipeId1)->second;
    relOp->Run(*inputPipe, *pipe, *selOp, *literal);
}

void SelectPipeRelOpNode::CleanUp() {
    relOp->WaitUntilDone();
}

void SumRelOpNode::Print() {
    cout << " *********** " << "\n";
    cout << "SUM operation" << "\n";
    RelOpNode::Print();
    cout << "\n";

    if (!computeMe) return;

    cout << "FUNCTION" << "\n";
    computeMe->Print(child1->outputSchema);
    cout << "\n";
    cout << "Distinct Function: " << distinctFunc << "\n";
    cout << "\n";
}

void SumRelOpNode::Execute(unordered_map<int, Pipe *> *pipes) {
    pipes->insert(pair<int, Pipe *>(outputPipeId, pipe));

    Pipe *inputPipe = pipes->find(inputPipeId1)->second;
    relOp->Run(*inputPipe, *pipe, *computeMe, distinctFunc);
}

void SumRelOpNode::CleanUp() {
    relOp->WaitUntilDone();
}

void ProjectRelOpNode::Print() {
    cout << " *********** " << "\n";
    cout << "PROJECT operation" << "\n";

    RelOpNode::Print();
    cout << "\n";
}

void ProjectRelOpNode::Execute(unordered_map<int, Pipe *> *pipes) {
    pipes->insert(pair<int, Pipe *>(outputPipeId, pipe));
    Pipe *inputPipe = pipes->find(inputPipeId1)->second;
    relOp->Run(*inputPipe, *pipe, keepMe, numAttsInput, numAttsOutput);
}

void ProjectRelOpNode::CleanUp() {
    relOp->WaitUntilDone();
}
