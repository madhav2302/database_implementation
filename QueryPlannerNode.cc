
#include "QueryPlannerNode.h"
#include <iostream>

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

void ProjectRelOpNode::Print() {
    cout << " *********** " << "\n";
    cout << "PROJECT operation" << "\n";

    RelOpNode::Print();
    cout << "\n";
}
