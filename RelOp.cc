#include "RelOp.h"
#include "RelOpStructs.h"
#include "BigQ.h"
#include <cmath>

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
    while (threadData->inFile->GetNext(temp, *threadData->selOp, *threadData->literal))
        threadData->outPipe->Insert(&temp);

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

    while (threadData->inPipe->Remove(&temp)) {
        temp.Project(threadData->keepMe, threadData->numAttsOutput, threadData->numAttsInput);
        threadData->outPipe->Insert(&temp);
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

    Record temp1, temp2;

    if (tempPipe.Remove(&temp1)) {
        while (tempPipe.Remove(&temp2)) {
            if (comp.Compare(&temp1, &temp2, &sortOrder) != 0) data->outPipe->Insert(&temp1);
            temp1.Consume(&temp2);
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

    Record temp;

    while (data->inPipe->Remove(&temp)) temp.WriteToFile(data->mySchema, data->outFile);

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

void Join::writeToFile(std::string fileName, Pipe *pipe) {
    File file;
    file.Open(0, const_cast<char *>(fileName.c_str()));
    int whichPage = 0;
    Page page;
    Record temp;
    int count = 0;
    while (pipe->Remove(&temp)) {
        count++;
        if (!page.Append(&temp)) {
            file.AddPage(&page, whichPage++);
            page.EmptyItOut();
            page.Append(&temp);
        }
    }

    file.AddPage(&page, whichPage++);
    file.Close();
}

void Join::NestedBlockJoin(Pipe *pipeL, Pipe *pipeR, int runlen, Pipe *out) {
    Record *outerRecord = new Record(), *innerRecord = new Record(), mergeRecord;
    std::string fileNameL = "tmp_file_l_" + randomFileName(), fileNameR = "tmp_file_r_" + randomFileName();
    writeToFile(fileNameL, pipeL);
    writeToFile(fileNameR, pipeR);

    File outer, inner;
    outer.Open(1, const_cast<char *>(fileNameL.c_str()));
    inner.Open(1, const_cast<char *> (fileNameR.c_str()));

    int leftCount = -1, rightCount = -1;
    int *attsToKeep = nullptr;

    for (int outerBlockNumber = 0;
         outerBlockNumber < std::ceil((outer.GetLength() - 1) / (double) runlen); outerBlockNumber++) {
        SingleRun outerBlock(&outer, runlen, outerBlockNumber);
        vector<Record *> outerRecords;
        while (outerBlock.GetFirst(innerRecord)) {
            outerRecords.push_back(innerRecord);
            outerRecord = new Record();
        }

        for (int innerBlockNumber = 0;
             innerBlockNumber < std::ceil((inner.GetLength() - 1) / (double) runlen); innerBlockNumber++) {
            SingleRun innerBlock(&inner, runlen, innerBlockNumber);
            vector<Record *> innerRecords;
            while (innerBlock.GetFirst(innerRecord)) {
                innerRecords.push_back(innerRecord);
                innerRecord = new Record();
            }

            for (Record *o : outerRecords) {
                for (Record *i : innerRecords) {
                    if (attsToKeep == nullptr) {
                        leftCount = o->NumberOfAtts();
                        rightCount = i->NumberOfAtts();
                        attsToKeep = new int[leftCount + rightCount];
                        int indexInArray = 0;
                        for (int i = 0; i < leftCount; i++) attsToKeep[indexInArray++] = i;
                        for (int i = 0; i < rightCount; i++) attsToKeep[indexInArray++] = i;
                    }

                    mergeRecord.MergeRecords(o, i, leftCount, rightCount, attsToKeep, leftCount + rightCount,
                                             leftCount);
                    out->Insert(&mergeRecord);
                }
            }
        }
    }

    outer.Close();
    inner.Close();
    remove(fileNameL.c_str());
    remove(fileNameR.c_str());
    out->ShutDown();
}

// TODO : Fix join if there are multiple records on first side of same values,
// It will make miss records for second table
void Join::ComparisonBasedJoin(Pipe *pipeL, Pipe *pipeR, OrderMaker *orderL, OrderMaker *orderR, Pipe *out) {
    ComparisonEngine comp;
    Record tempLeft, tempRight;
    int *attsToKeep = nullptr;
    int rightIsPresent = 0;
    int leftCount = -1, rightCount = -1;

    while (pipeL->Remove(&tempLeft)) {
        while (rightIsPresent || pipeR->Remove(&tempRight)) {
            int comparisionResult = comp.Compare(&tempLeft, orderL, &tempRight, orderR);

            if (comparisionResult == 0) {
                if (attsToKeep == nullptr) {
                    leftCount = tempLeft.NumberOfAtts();
                    rightCount = tempRight.NumberOfAtts();
                    attsToKeep = new int[leftCount + rightCount];
                    int indexInArray = 0;
                    for (int i = 0; i < leftCount; i++) attsToKeep[indexInArray++] = i;
                    for (int i = 0; i < rightCount; i++) attsToKeep[indexInArray++] = i;
                }

                Record tempMerge;
                tempMerge.MergeRecords(&tempLeft, &tempRight, leftCount, rightCount, attsToKeep, leftCount + rightCount,
                                       leftCount);
                out->Insert(&tempMerge);
            } else if (comparisionResult < 0) {
                rightIsPresent = 1;
                break;
            }
            rightIsPresent = 0;
        }
    }
    out->ShutDown();
}

void *Join::ThreadMethod(void *d) {
    auto *data = (RelOpJoinData *) d;

    OrderMaker left, right;
    data->selOp->GetSortOrders(left, right);

    Pipe leftData(100), rightData(100);
    BigQ bigQL(*data->inPipeL, leftData, left, data->runLen), bigQR(*data->inPipeR, rightData, right, data->runLen);

    if (left.numAtts == 0) NestedBlockJoin(&leftData, &rightData, data->runLen, data->outPipe);
    else ComparisonBasedJoin(&leftData, &rightData, &left, &right, data->outPipe);

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
    for (int index = 0; index < o.numAtts; index++) output += r.GetAtt(o.whichAtts[index], o.whichTypes[index]) + "|";
    return output;
}

void *GroupBy::ThreadMethod(void *d) {
    ComparisonEngine comp;

    auto *data = (RelOpGroupByData *) d;
    Attribute atts[1 + data->groupAtts->numAtts];
    Attribute x = {"attNum", Double};
    atts[0] = x;
    for (int i = 0; i < data->groupAtts->numAtts; i++) {
        Attribute y = {"att", data->groupAtts->whichTypes[i]};
        atts[1 + i] = y;
    }
    Schema outSchema("out_schema", 1 + data->groupAtts->numAtts, atts);
    Record outPipeTemp;

    Pipe tempPipe(100);
    BigQ bigQ(*data->inPipe, tempPipe, *data->groupAtts, data->runLen);

    Record temp1, temp2;

    if (!tempPipe.Remove(&temp1)) {
        data->outPipe->ShutDown();
        return nullptr;
    }

    int intResult = 0, tempIntResult;
    double doubleResult = 0.0, tempDoubleResult;

    while (tempPipe.Remove(&temp2)) {
        tempIntResult = 0;
        tempDoubleResult = 0.0;

        int comparision = comp.Compare(&temp1, &temp2, data->groupAtts);

        Type type = data->computeMe->Apply(temp1, tempIntResult, tempDoubleResult);
        intResult += tempIntResult;
        doubleResult += tempDoubleResult;

        if (comparision == 0) {
            temp1.Consume(&temp2);
            continue;
        }

        std::string output = type == Int ? std::to_string(intResult) : std::to_string(intResult + doubleResult) + "|";
        atts->myType = type;
        outPipeTemp.ComposeRecord(&outSchema, AppendOrderAtts(output, *data->groupAtts, temp1).c_str());
        data->outPipe->Insert(&outPipeTemp);

        intResult = 0;
        doubleResult = 0.0;
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
    outPipeTemp.ComposeRecord(&outSchema, AppendOrderAtts(output, *data->groupAtts, temp1).c_str());
    data->outPipe->Insert(&outPipeTemp);

    data->outPipe->ShutDown();
    return nullptr;
}