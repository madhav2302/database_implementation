#include "RelOp.h"
#include "RelOpStructs.h"
#include <iostream>
#include "BigQ.h"
#include "SortedDBFile.h"

void *SelectFileFunction(void *data);

void *SelectPipeFunction(void *data);

void *ProjectFunction(void *data);

void *SumFunction(void *data);

void *DuplicateRemoveFunction(void *data);

void *WriteOutFunction(void *data);

void *JoinFunction(void *data);

void *GroupByFunction(void *data);

void WriteValueToFile(Type type, FILE *outRecFile, int intResult, double doubleResult);

void SelectFile::Run(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal) {
    auto *data = new RelOpSelectFileData();
    data->inFile = &inFile;
    data->outPipe = &outPipe;
    data->selOp = &selOp;
    data->literal = &literal;
    pthread_create(&thread, nullptr, SelectFileFunction, (void *) data);
}

void *SelectFileFunction(void *data) {
    auto *threadData = (RelOpSelectFileData *) data;

    Record temp;
    while (threadData->inFile->GetNext(temp, *threadData->selOp, *threadData->literal)) {
        threadData->outPipe->Insert(&temp);
    }
    threadData->outPipe->ShutDown();
    return nullptr;
}

void SelectPipe::Run(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal) {
    auto *data = new RelOpSelectPipeData();
    data->inPipe = &inPipe;
    data->outPipe = &outPipe;
    data->selOp = &selOp;
    data->literal = &literal;
    pthread_create(&thread, nullptr, SelectPipeFunction, (void *) data);
}

void *SelectPipeFunction(void *data) {
    auto *threadData = (RelOpSelectPipeData *) data;
    ComparisonEngine comp;
    Record temp;
    while (threadData->inPipe->Remove(&temp)) {
        if (comp.Compare(&temp, threadData->literal, threadData->selOp))
            threadData->outPipe->Insert(&temp);
    }
    threadData->outPipe->ShutDown();
    return nullptr;
}

void Project::Run(Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput) {
    auto *data = new RelOpProjectData();
    data->inPipe = &inPipe;
    data->outPipe = &outPipe;
    data->keepMe = keepMe;
    data->numAttsInput = numAttsInput;
    data->numAttsOutput = numAttsOutput;
    pthread_create(&thread, nullptr, ProjectFunction, (void *) data);
}

void *ProjectFunction(void *data) {
    RelOpProjectData *threadData = (RelOpProjectData *) data;
    Record temp;

    int count = 0;

    while (threadData->inPipe->Remove(&temp)) {
        temp.Project(threadData->keepMe, threadData->numAttsOutput, threadData->numAttsInput);
        threadData->outPipe->Insert(&temp);
        count++;
    }

    threadData->outPipe->ShutDown();
    return nullptr;
}

void Sum::Run(Pipe &inPipe, Pipe &outPipe, Function &computeMe) {
    auto *data = new RelOpSumData();
    data->inPipe = &inPipe;
    data->outPipe = &outPipe;
    data->computeMe = &computeMe;
    pthread_create(&thread, nullptr, SumFunction, (void *) data);
}

void *SumFunction(void *d) {
    auto *data = (RelOpSumData *) d;

    Type type = Int;

    int intResult = 0;
    double doubleResult = 0.0;

    Record temp;
    while (data->inPipe->Remove(&temp)) {
        int tempIntResult = 0;
        double tempDoubleResult = 0.0;
        Type resultType = data->computeMe->Apply(temp, tempIntResult, tempDoubleResult);
        if (resultType == Double) type = Double;

        intResult += tempIntResult;
        doubleResult += tempDoubleResult;
    }

    FILE *outRecFile = fopen("tmp_relop_sum.tmp", "w");
    WriteValueToFile(type, outRecFile, intResult, doubleResult);
    fclose(outRecFile);

    outRecFile = fopen("tmp_relop_sum.tmp", "r");

    Attribute att = {"att1", type};
    Schema outSchema("out_schema", 1, &att);
    temp.SuckNextRecord(&outSchema, outRecFile);
    data->outPipe->Insert(&temp);

    fclose(outRecFile);
    remove("tmp_relop_sum.tmp");

    data->outPipe->ShutDown();
    return nullptr;
}

void DuplicateRemoval::Run(Pipe &inPipe, Pipe &outPipe, Schema &mySchema) {
    auto *data = new RelOpDuplicateRemovalData();
    data->inPipe = &inPipe;
    data->outPipe = &outPipe;
    data->mySchema = &mySchema;
    data->runLen = runLen;
    pthread_create(&thread, nullptr, DuplicateRemoveFunction, (void *) data);
}

void *DuplicateRemoveFunction(void *d) {
    ComparisonEngine comp;
    auto *data = (RelOpDuplicateRemovalData *) d;

    OrderMaker sortOrder;
    sortOrder.numAtts = data->mySchema->GetNumAtts();

    for (int i = 0; i < sortOrder.numAtts; i++) {
        sortOrder.whichAtts[i] = i;
        sortOrder.whichTypes[i] = (data->mySchema->GetAtts() + i)->myType;
    }

    Pipe tempPipe(100);
    BigQ bigQ(*data->inPipe, tempPipe, sortOrder, data->runLen);

    Record temp1;
    Record temp2;

    int recordFound = tempPipe.Remove(&temp1);

    if (recordFound) {
        while (tempPipe.Remove(&temp2)) {
            if (comp.Compare(&temp1, &temp2, &sortOrder) != 0) {
                data->outPipe->Insert(&temp1);
                temp1.Consume(&temp2);
            }
        }

        data->outPipe->Insert(&temp1);
    }

    data->outPipe->ShutDown();
    return nullptr;
}

void WriteOut::Run(Pipe &inPipe, FILE *outFile, Schema &mySchema) {
    auto *data = new RelOpWriteOutData();
    data->inPipe = &inPipe;
    data->outFile = outFile;
    data->mySchema = &mySchema;
    pthread_create(&thread, nullptr, WriteOutFunction, (void *) data);
}

void *WriteOutFunction(void *d) {
    auto *data = (RelOpWriteOutData *) d;

    int count = 0;
    Record temp;

    while (data->inPipe->Remove(&temp)) {
        temp.WriteToFile(data->mySchema, data->outFile);
        count++;
    }

    fclose(data->outFile);
    return nullptr;
}

void Join::Run(Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal) {
    auto *data = new RelOpJoinData();
    data->inPipeL = &inPipeL;
    data->inPipeR = &inPipeR;
    data->outPipe = &outPipe;
    data->selOp = &selOp;
    data->literal = &literal;
    data->runLen = runLen;
    pthread_create(&thread, nullptr, JoinFunction, (void *) data);
}

// TODO : Think about using BigQ instead of SortedDBFile
void *JoinFunction(void *d) {
    auto *data = (RelOpJoinData *) d;
    ComparisonEngine comp;
    Record tempLeft;
    Record tempRight;

    OrderMaker left;
    OrderMaker right;
    data->selOp->GetSortOrders(left, right);

    SortInfo leftSortInfo(&left, data->runLen);

    GenericDBFile *leftData = new SortedDBFile();
    leftData->Create("left_sorted_tmp.tmp", sorted, &leftSortInfo);

    while (data->inPipeL->Remove(&tempLeft)) {
        leftData->Add(tempLeft);
    }

    leftData->MoveFirst();

    GenericDBFile *rightData = new SortedDBFile();
    SortInfo rightSortInfo(&right, data->runLen);
    rightData->Create("right_sorted_tmp.tmp", sorted, &rightSortInfo);

    while (data->inPipeR->Remove(&tempRight)) {
        rightData->Add(tempRight);
    }

    rightData->MoveFirst();

    int *attsToKeep = nullptr;
    int rightIsPresent = 0;

    int count = 0;

    while (leftData->GetNext(tempLeft)) {
        count++;
        while (rightIsPresent || rightData->GetNext(tempRight)) {
            int comparisionResult = comp.Compare(&tempLeft, &left, &tempRight, &right);

            if (comparisionResult == 0) {
                int leftCount = ((int *) tempLeft.bits)[1] / sizeof(int) - 1;
                int rightCount = ((int *) tempRight.bits)[1] / sizeof(int) - 1;

                if (attsToKeep == nullptr) {
                    attsToKeep = new int[leftCount + rightCount];

                    int indexInArray = 0;
                    for (int i = 0; i < leftCount; i++) {
                        attsToKeep[indexInArray++] = i;
                    }

                    for (int i = 0; i < rightCount; i++) {
                        attsToKeep[indexInArray++] = i;
                    }
                }

                Record tempMerge;
                tempMerge.MergeRecords(&tempLeft, &tempRight, leftCount, rightCount, attsToKeep, leftCount + rightCount,
                                       leftCount);
                data->outPipe->Insert(&tempMerge);
            } else if (comparisionResult < 0) {
                rightIsPresent = 1;
                break;
            }
            rightIsPresent = 0;
        }

        if (count % 10000 == 0) {
            cout << "Completed : " << count << " records.\n";
        }
    }

    leftData->Close();
    rightData->Close();

    remove("right_sorted_tmp.tmp");
    remove("right_sorted_tmp.tmp.metadata");
    remove("left_sorted_tmp.tmp");
    remove("left_sorted_tmp.tmp.metadata");

    data->outPipe->ShutDown();
    return nullptr;
}

void GroupBy::Run(Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe) {
    auto *data = new RelOpGroupByData();
    data->inPipe = &inPipe;
    data->outPipe = &outPipe;
    data->groupAtts = &groupAtts;
    data->computeMe = &computeMe;
    data->runLen = runLen;

    pthread_create(&thread, nullptr, GroupByFunction, (void *) data);
}

void *GroupByFunction(void *d) {
    ComparisonEngine comp;

    auto *data = (RelOpGroupByData *) d;

    Pipe tempPipe(100);
    BigQ bigQ(*data->inPipe, tempPipe, *data->groupAtts, data->runLen);

    Record temp1;
    Record temp2;

    int recordFound = tempPipe.Remove(&temp1);

    if (!recordFound) {
        data->outPipe->ShutDown();
        return nullptr;
    }

    FILE *outRecFile = fopen("tmp_relop_group_by.tmp", "w");

    int intResult = 0;
    double doubleResult = 0.0;

    int tempIntResult;
    double tempDoubleResult;

    while (tempPipe.Remove(&temp2)) {
        tempIntResult = 0;
        tempDoubleResult = 0.0;

        int comparision = comp.Compare(&temp1, &temp2, data->groupAtts);

        Type type = data->computeMe->Apply(temp1, tempIntResult, tempDoubleResult);
        intResult += tempIntResult;
        doubleResult += tempDoubleResult;

        if (comparision != 0) {
            WriteValueToFile(type, outRecFile, intResult, doubleResult);
            intResult = 0;
            doubleResult = 0.0;
        }
        temp1.Consume(&temp2);
    }

    tempIntResult = 0;
    tempDoubleResult = 0.0;
    Type type = data->computeMe->Apply(temp1, tempIntResult, tempDoubleResult);
    intResult += tempIntResult;
    doubleResult += tempDoubleResult;
    WriteValueToFile(type, outRecFile, intResult, doubleResult);

    fclose(outRecFile);

    outRecFile = fopen("tmp_relop_group_by.tmp", "r");
    Attribute att = {"att1", type};
    Schema outSchema("out_schema", 1, &att);
    Record temp;
    while (temp.SuckNextRecord(&outSchema, outRecFile)) {
        data->outPipe->Insert(&temp);
    }

    fclose(outRecFile);
    remove("tmp_relop_group_by.tmp");

    data->outPipe->ShutDown();
    return nullptr;
}

void WriteValueToFile(Type type, FILE *outRecFile, int intResult, double doubleResult) {
    if (type == Int) {
        fprintf(outRecFile, "%d|\n", intResult);
    } else {
        double total = intResult + doubleResult;
        fprintf(outRecFile, "%f|\n", total);
    }
}
