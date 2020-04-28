#include "RelOp.h"
#include "RelOpStructs.h"
#include "BigQ.h"
#include <cmath>

void RelationalOp::WaitUntilDone() {
    pthread_join(thread, nullptr);
//    cerr << operation() << " Completed\n";
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
    if (threadData->selOp) {
        while (threadData->inFile->GetNext(temp, *threadData->selOp, *threadData->literal))
            threadData->outPipe->Insert(&temp);

    } else {
        while (threadData->inFile->GetNext(temp)) threadData->outPipe->Insert(&temp);
    }

    threadData->outPipe->ShutDown();
    return nullptr;
}

string SelectFile::operation() {
    return "SelecteFile";
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

string SelectPipe::operation() {
    return "SelectPipe";
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

string Project::operation() {
    return "Project";
}

void Sum::Run(Pipe &inPipe, Pipe &outPipe, Function &computeMe, bool distinctFunc) {
    auto *data = new RelOpSumData();
    data->inPipe = &inPipe;
    data->outPipe = &outPipe;
    data->computeMe = &computeMe;
    data->distinctFunc = distinctFunc;
    pthread_create(&thread, nullptr, ThreadMethod, (void *) data);
}

void DistinctSum(RelOpSumData *data, Schema *outSchema) {
    DuplicateRemoval duplicateRemoval;
    Pipe *duplicateRemovalInPipe = new Pipe(100);
    Pipe *duplicateRemovalOutPipe = new Pipe(100);
    duplicateRemoval.Run(*duplicateRemovalInPipe, *duplicateRemovalOutPipe, *outSchema);

    Record temp;
    while (data->inPipe->Remove(&temp)) {
        int tempIntResult = 0;
        double tempDoubleResult = 0.0;
        data->computeMe->Apply(temp, tempIntResult, tempDoubleResult);

        std::string output = std::to_string(tempIntResult + tempDoubleResult) + "|";

        temp.ComposeRecord(outSchema, const_cast<char *>(output.c_str()));
        duplicateRemovalInPipe->Insert(&temp);
    }
    duplicateRemovalInPipe->ShutDown();

    double result = 0.0;
    while (duplicateRemovalOutPipe->Remove(&temp)) result += std::stod(temp.GetAtt(0, Double));

    std::string output = std::to_string(result) + "|";

    temp.ComposeRecord(outSchema, const_cast<char *>(output.c_str()));
    data->outPipe->Insert(&temp);
    data->outPipe->ShutDown();
}

void SimpleSum(RelOpSumData *data, Schema *outSchema) {
    int intResult = 0;
    double doubleResult = 0.0;

    Record temp;
    while (data->inPipe->Remove(&temp)) {
        int tempIntResult = 0;
        double tempDoubleResult = 0.0;
        data->computeMe->Apply(temp, tempIntResult, tempDoubleResult);

        intResult += tempIntResult;
        doubleResult += tempDoubleResult;
    }

    std::string output = std::to_string(intResult + doubleResult) + "|";

    temp.ComposeRecord(outSchema, const_cast<char *>(output.c_str()));
    data->outPipe->Insert(&temp);
    data->outPipe->ShutDown();
}

void *Sum::ThreadMethod(void *d) {
    auto *data = (RelOpSumData *) d;
    Attribute att = {"att1", Double};
    Schema outSchema("out_schema", 1, &att);

    if (data->distinctFunc) DistinctSum(data, &outSchema);
    else SimpleSum(data, &outSchema);

    return nullptr;
}

string Sum::operation() {
    return "Sum";
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

string DuplicateRemoval::operation() {
    return "DuplicateRemoval";
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

string WriteOut::operation() {
    return "WriteOut";
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
                        for (int index = 0; index < leftCount; index++) attsToKeep[indexInArray++] = index;
                        for (int index = 0; index < rightCount; index++) attsToKeep[indexInArray++] = index;
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

void Join::ComparisonBasedJoin(Pipe *pipeL, Pipe *pipeR, OrderMaker *orderL, OrderMaker *orderR, Pipe *out) {
    ComparisonEngine comp;
    Record *tempLeft = new Record(), *tempRight = new Record(), *firstLeft = nullptr;
    int *attsToKeep = nullptr;
    int leftIsPresent = 0, rightIsPresent = 0;
    int leftCount = -1, rightCount = -1;

    vector<Record *> leftRecords, rightRecords;
    while (leftIsPresent || pipeL->Remove(tempLeft)) {
        leftRecords.push_back(tempLeft);
        leftIsPresent = 0;

        firstLeft = new Record();
        firstLeft->Copy(tempLeft);

        tempLeft = new Record();
        while (pipeL->Remove(tempLeft)) {
            if (comp.Compare(firstLeft, tempLeft, orderL) == 0) {
                leftRecords.push_back(tempLeft);
                tempLeft = new Record();
            } else {
                leftIsPresent = 1;
                break;
            }
        }

        while (rightIsPresent || pipeR->Remove(tempRight)) {
            rightIsPresent = 0;
            int comparisonResult = comp.Compare(tempRight, orderR, firstLeft, orderL);
            if (comparisonResult == 0) {
                rightRecords.push_back(tempRight);
                tempRight = new Record();
            } else if (comparisonResult < 0) {
                tempRight = new Record();
            } else {
                rightIsPresent = 1;
                break;
            }
        }

        for (Record *l : leftRecords) {
            for (Record *r : rightRecords) {
                if (attsToKeep == nullptr) {
                    leftCount = firstLeft->NumberOfAtts();
                    rightCount = tempRight->NumberOfAtts();
                    attsToKeep = new int[leftCount + rightCount];
                    int indexInArray = 0;
                    for (int i = 0; i < leftCount; i++) attsToKeep[indexInArray++] = i;
                    for (int i = 0; i < rightCount; i++) attsToKeep[indexInArray++] = i;
                }

                Record tempMerge;
                tempMerge.MergeRecords(l, r, leftCount, rightCount, attsToKeep, leftCount + rightCount, leftCount);
                out->Insert(&tempMerge);
            }
        }

        leftRecords.clear();
        rightRecords.clear();
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

string Join::operation() {
    return "Join";
}

void GroupBy::Run(Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe, bool distinctFunc) {
    auto *data = new RelOpGroupByData();
    data->inPipe = &inPipe;
    data->outPipe = &outPipe;
    data->groupAtts = &groupAtts;
    data->computeMe = &computeMe;
    data->runLen = runLen;
    data->distinctFunc = distinctFunc;

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

    Sum *sum = new Sum();
    Pipe *sumInPipe = new Pipe(100), *sumOutPipe = new Pipe(100);
    sum->Run(*sumInPipe, *sumOutPipe, *data->computeMe, data->distinctFunc);

    while (tempPipe.Remove(&temp2)) {
        int comparision = comp.Compare(&temp1, &temp2, data->groupAtts);
        Record temp1Copy;
        temp1Copy.Copy(&temp1);

        sumInPipe->Insert(&temp1);

        if (comparision == 0) {
            temp1.Consume(&temp2);
            continue;
        }
        sumInPipe->ShutDown();

        Record outTemp;
        sumOutPipe->Remove(&outTemp);

        std::string output = outTemp.GetAtt(0, Double) + "|";
        outPipeTemp.ComposeRecord(&outSchema, AppendOrderAtts(output, *data->groupAtts, temp1Copy).c_str());
        data->outPipe->Insert(&outPipeTemp);

        temp1.Consume(&temp2);

        sum = new Sum();
        sumInPipe = new Pipe(100);
        sumOutPipe = new Pipe(100);
        sum->Run(*sumInPipe, *sumOutPipe, *data->computeMe, data->distinctFunc);
    }

    Record temp1Copy;
    temp1Copy.Copy(&temp1);
    sumInPipe->Insert(&temp1);
    sumInPipe->ShutDown();

    Record outTemp;
    sumOutPipe->Remove(&outTemp);

    std::string output = outTemp.GetAtt(0, Double) + "|";
    outPipeTemp.ComposeRecord(&outSchema, AppendOrderAtts(output, *data->groupAtts, temp1Copy).c_str());
    data->outPipe->Insert(&outPipeTemp);

    data->outPipe->ShutDown();
    return nullptr;
}

string GroupBy::operation() {
    return "GroupBy";
}
