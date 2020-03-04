#include "RelOp.h"
#include "RelOpStructs.h"
#include <iostream>

void SelectFile::Run(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal) {
    RelOpSelectFileData *data = new RelOpSelectFileData(inFile, outPipe, selOp, literal);
    pthread_create(&thread, NULL, SelectFileFunction, (void *) data);
}

void SelectFile::WaitUntilDone() {
    pthread_join(thread, NULL);
}

void SelectFile::Use_n_Pages(int runlen) {

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
}

void SelectPipe::Use_n_Pages(int runlen) {

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
}

void Project::Use_n_Pages(int n) {

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
}

void Sum::Use_n_Pages(int n) {

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
