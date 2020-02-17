#ifndef BIGQ_H
#define BIGQ_H

#include <pthread.h>
#include <iostream>
#include "Pipe.h"
#include "File.h"
#include "Record.h"

using namespace std;

class BigQ {

private:
    File *file = new File();
    Page *page;
    off_t writePage;


    void SortRun(vector<Page *> &pages, Pipe &in, Pipe &out, OrderMaker *sortorder, int runlen);

public:

    BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);

    ~BigQ();
};

#endif
