#include "RelOp.h"
#include "RelOpStructs.h"
#include <iostream>
#include "BigQ.h"

void SelectFile::Run(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal) {
    RelOpSelectFileData *data = new RelOpSelectFileData(inFile, outPipe, selOp, literal);
    pthread_create(&thread, NULL, SelectFileFunction, (void *) data);
}

void SelectFile::WaitUntilDone() {
    pthread_join(thread, NULL);
    cerr << "Select File Completed\n";
}

void *SelectFileFunction(void *data) {
    RelOpSelectFileData *threadData = (RelOpSelectFileData *) data;

    Record temp;
    while (threadData->inFile.GetNext(temp, threadData->selOp, threadData->literal)) {
        threadData->outPipe.Insert(&temp);
    }
    threadData->outPipe.ShutDown();
//    return nullptr;
}

void SelectPipe::Run(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal) {
    RelOpSelectPipeData *data = new RelOpSelectPipeData(inPipe, outPipe, selOp, literal);
    pthread_create(&thread, NULL, SelectPipeFunction, (void *) data);
}

void SelectPipe::WaitUntilDone() {
    pthread_join(thread, NULL);
    cerr << "Select Pipe Completed\n";
}

void *SelectPipeFunction(void *data) {
    RelOpSelectPipeData *threadData = (RelOpSelectPipeData *) data;
    ComparisonEngine comp;
    Record temp;
    while (threadData->inPipe.Remove(&temp)) {
        if (comp.Compare(&temp, &threadData->literal, &threadData->selOp))
            threadData->outPipe.Insert(&temp);
    }
    threadData->outPipe.ShutDown();
//    return nullptr;
}

void Project::Run(Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput) {
    RelOpProjectData *data = new RelOpProjectData(inPipe, outPipe, keepMe, numAttsInput, numAttsOutput);
    pthread_create(&thread, NULL, ProjectFunction, (void *) data);
}

void Project::WaitUntilDone() {
    pthread_join(thread, NULL);
    cerr << "Project Completed\n";
}

void *ProjectFunction(void *data) {
    RelOpProjectData *threadData = (RelOpProjectData *) data;
    Record temp;

    int count = 0;

    while (threadData->inPipe.Remove(&temp)) {
        temp.Project(threadData->keepMe, threadData->numAttsOutput, threadData->numAttsInput);
        threadData->outPipe.Insert(&temp);
        count++;
    }

    cout << "Found " << count << '\n';

    threadData->outPipe.ShutDown();
}

void Sum::Run(Pipe &inPipe, Pipe &outPipe, Function &computeMe) {
    RelOpSumData *data = new RelOpSumData(inPipe, outPipe, computeMe);
    pthread_create(&thread, NULL, SumFunction, (void *) data);
}

void Sum::WaitUntilDone() {
    pthread_join(thread, NULL);
    cerr << "Sum Completed\n";
}

void *SumFunction(void *data) {
    RelOpSumData *threadData = (RelOpSumData *) data;

    Type type = Int;

    int intResult = 0;
    double doubleResult = 0.0;

    Record temp;
    while (threadData->inPipe.Remove(&temp)) {
        int tempIntResult = 0;
        double tempDoubleResult = 0.0;
        Type resultType = threadData->computeMe.Apply(temp, tempIntResult, tempDoubleResult);
        if (resultType == Double) type = Double;

        intResult += tempIntResult;
        doubleResult += tempDoubleResult;
    }

    FILE *outRecFile = fopen("tmp_relop_sum", "w");
    if (type == Int) {
        fprintf(outRecFile, "%d|", intResult);
    } else {
        double total = intResult + doubleResult;
        fprintf(outRecFile, "%f|", total);
    }
    fclose(outRecFile);

    outRecFile = fopen("tmp_relop_sum", "r");

    Attribute att = {"att1", type};
    Schema outSchema("out_schema", 1, &att);
    temp.SuckNextRecord(&outSchema, outRecFile);
    threadData->outPipe.Insert(&temp);

    fclose(outRecFile);
    remove("tmp_relop_sum");

    threadData->outPipe.ShutDown();
}

void DuplicateRemoval::Run(Pipe &inPipe, Pipe &outPipe, Schema &mySchema) {
    RelOpDuplicateRemovalData *data = new RelOpDuplicateRemovalData(inPipe, outPipe, mySchema, runLen);
    pthread_create(&thread, NULL, DuplicateRemoveFunction, (void *) data);
}

void DuplicateRemoval::WaitUntilDone() {
    pthread_join(thread, NULL);
    cerr << "Duplicate Removal Completed\n";
}

void DuplicateRemoval::Use_n_Pages(int n) {
    this->runLen = n;
}

void *DuplicateRemoveFunction(void *d) {
    ComparisonEngine comp;
    RelOpDuplicateRemovalData *data = (RelOpDuplicateRemovalData *) d;

    OrderMaker sortOrder;
    sortOrder.numAtts = data->mySchema.GetNumAtts();

    for (int i = 0; i < sortOrder.numAtts; i++) {
        sortOrder.whichAtts[i] = i;
        sortOrder.whichTypes[i] = (data->mySchema.GetAtts() + i)->myType;
    }

    Pipe tempPipe(100);
    BigQ bigQ(data->inPipe, tempPipe, sortOrder, data->runLen);

    Record temp1;
    Record temp2;

    int recordFound = tempPipe.Remove(&temp1);

    if (recordFound) {
        while (tempPipe.Remove(&temp2)) {
            if (comp.Compare(&temp1, &temp2, &sortOrder) != 0) {
                data->outPipe.Insert(&temp1);
                temp1.Consume(&temp2);
            }
        }
    }

    data->outPipe.Insert(&temp1);
    data->outPipe.ShutDown();
}

void WriteOut::Run(Pipe &inPipe, FILE *outFile, Schema &mySchema) {
    RelOpWriteOutData *data = new RelOpWriteOutData(inPipe, outFile, mySchema);
    pthread_create(&thread, NULL, WriteOutFunction, (void *) data);
}

void WriteOut::WaitUntilDone() {
    pthread_join(thread, NULL);
    cerr << "WriteOut Completed\n";
}

void *WriteOutFunction(void *d) {
    RelOpWriteOutData *data = (RelOpWriteOutData *) d;

    int count = 0;
    Record temp;

    while (data->inPipe.Remove(&temp)) {
        temp.WriteToFile(&data->mySchema, data->outFile);
        count++;
    }

    cout << "Number of records : " << count << '\n';
    fclose(data->outFile);
}

