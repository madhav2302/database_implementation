#include "RelOp.h"
#include "RelOpStructs.h"
#include <iostream>
#include "BigQ.h"
#include "SortedDBFile.h"

void RelationalOp::WaitUntilDone() {
    pthread_join(thread, nullptr);
}

void RelationalOp::Use_n_Pages(int n) {
    runLen = n;
}

void SelectFile::Run(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal) {
    auto *data = new RelOpSelectFileData();
    data->inFile = &inFile;
    data->outPipe = &outPipe;
    data->selOp = &selOp;
    data->literal = &literal;
    pthread_create(&thread, nullptr, ThreadMethod, (void *) data);
}

void *SelectFile::ThreadMethod(void *d) {
    auto *threadData = (RelOpSelectFileData *) d;

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
    pthread_create(&thread, nullptr, ThreadMethod, (void *) data);
}

void *SelectPipe::ThreadMethod(void *data) {
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
    pthread_create(&thread, nullptr, ThreadMethod, (void *) data);
}

void *Project::ThreadMethod(void *data) {
    auto *threadData = (RelOpProjectData *) data;
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
    pthread_create(&thread, nullptr, ThreadMethod, (void *) data);
}

void *Sum::ThreadMethod(void *d) {
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

    std::string output = type == Int ? std::to_string(intResult) : std::to_string(intResult + doubleResult) + "|";
    Attribute att = {"att1", type};
    Schema outSchema("out_schema", 1, &att);
    temp.ComposeRecord(&outSchema, const_cast<char *>(output.c_str()));
    data->outPipe->Insert(&temp);

    data->outPipe->ShutDown();
    return nullptr;
}

void DuplicateRemoval::Run(Pipe &inPipe, Pipe &outPipe, Schema &mySchema) {
    auto *data = new RelOpDuplicateRemovalData();
    data->inPipe = &inPipe;
    data->outPipe = &outPipe;
    data->mySchema = &mySchema;
    data->runLen = runLen;
    pthread_create(&thread, nullptr, ThreadMethod, (void *) data);
}

void *DuplicateRemoval::ThreadMethod(void *d) {
    ComparisonEngine comp;
    auto *data = (RelOpDuplicateRemovalData *) d;

    OrderMaker sortOrder(data->mySchema);

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
    pthread_create(&thread, nullptr, ThreadMethod, (void *) data);
}

void *WriteOut::ThreadMethod(void *d) {
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
    pthread_create(&thread, nullptr, ThreadMethod, (void *) data);
}

// TODO : Think about using BigQ instead of SortedDBFile
void *Join::ThreadMethod(void *d) {
    auto *data = (RelOpJoinData *) d;
    ComparisonEngine comp;
    Record tempLeft;
    Record tempRight;

    OrderMaker left;
    OrderMaker right;
    data->selOp->GetSortOrders(left, right);

    SortInfo leftSortInfo(&left, data->runLen);

    GenericDBFile *leftData = new SortedDBFile();
    leftData->Create("tmp_left_sorted.tmp", sorted, &leftSortInfo);

    while (data->inPipeL->Remove(&tempLeft)) {
        leftData->Add(tempLeft);
    }

    leftData->MoveFirst();

    GenericDBFile *rightData = new SortedDBFile();
    SortInfo rightSortInfo(&right, data->runLen);
    rightData->Create("tmp_right_sorted.tmp", sorted, &rightSortInfo);

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
                int leftCount = tempLeft.NumberOfAtts();
                int rightCount = tempRight.NumberOfAtts();

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

    remove("tmp_right_sorted.tmp");
    remove("tmp_right_sorted.tmp.metadata");
    remove("tmp_left_sorted.tmp");
    remove("tmp_left_sorted.tmp.metadata");

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

    pthread_create(&thread, nullptr, ThreadMethod, (void *) data);
}

std::string AppendOrderAtts(std::string output, OrderMaker &o, Record &r) {
    for (int index = 0; index < o.numAtts; index++) {
        output += r.GetAtt(o.whichAtts[index], o.whichTypes[index]) + "|";
    }
    return output;
}

void *GroupBy::ThreadMethod(void *d) {
    ComparisonEngine comp;

    auto *data = (RelOpGroupByData *) d;
    Attribute atts[1 + data->groupAtts->numAtts];
    Attribute x ={"attNum", Double};
    atts[0] = x;
    for (int i = 0; i < data->groupAtts->numAtts; i++) {
        Attribute y = {"att", data->groupAtts->whichTypes[i]};
        atts[1 + i] = y;
    }
    Schema outSchema("out_schema", 1 + data->groupAtts->numAtts, atts);
    Record temp;

    Pipe tempPipe(100);
    BigQ bigQ(*data->inPipe, tempPipe, *data->groupAtts, data->runLen);

    Record temp1;
    Record temp2;

    int recordFound = tempPipe.Remove(&temp1);

    if (!recordFound) {
        data->outPipe->ShutDown();
        return nullptr;
    }

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
            std::string output =
                    type == Int ? std::to_string(intResult) : std::to_string(intResult + doubleResult) + "|";
            atts->myType = type;
            temp.ComposeRecord(&outSchema, AppendOrderAtts(output, *data->groupAtts, temp1).c_str());
            data->outPipe->Insert(&temp);

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

    std::string output =
            type == Int ? std::to_string(intResult) : std::to_string(intResult + doubleResult) + "|";
    atts->myType = type;
    temp.ComposeRecord(&outSchema, AppendOrderAtts(output, *data->groupAtts, temp1).c_str());
    data->outPipe->Insert(&temp);

    data->outPipe->ShutDown();
    return nullptr;
}