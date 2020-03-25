#include "gtest/gtest.h"
#include "Schema.h"
#include "Pipe.h"
#include "a3-test.h"
#include "RelOp.h"
#include "RelOpStructs.h"
#include "Comparison.h"
#include <fstream>

Attribute leftId = {"left_id", Int};
Attribute left1 = {"left_1", String};
Attribute left2 = {"left_1", String};
Attribute leftAtts[] = {leftId, left1, left2};
Schema left_schema("left_schema", 3, leftAtts);

Attribute rightId = {"right_id", Int};
Attribute right1 = {"right_1", Int};
Attribute right2 = {"right_2", String};
Attribute right3 = {"right_3", String};
Attribute right4 = {"right_4", String};
Attribute rightAtts[] = {rightId, right1, right2, right3, right4};
Schema right_schema("right_schema", 5, rightAtts);


bool fileExists(std::string fileName) {
    ifstream f(fileName.c_str());
    return f.good();
}

void InsertRightRecords(Pipe *in, int number) {
    Record record;
    for (int i = 1; i <= number; i++) {
        std::string iS = std::to_string(i);
        std::string recordString = iS + "|" + iS + "|right_2_" + iS + "|right_3_" + iS + "|right_4_" + iS + "|";
        record.ComposeRecord(&right_schema, recordString.c_str());
        in->Insert(&record);
    }
    in->ShutDown();
}

void InsertLeftRecords(Pipe *in, int number) {
    Record record;
    for (int i = 1; i <= number; i++) {
        std::string iS = std::to_string(i);
        std::string recordString = iS + "|left_1_" + iS + "|left_2_" + iS + "|";
        record.ComposeRecord(&left_schema, recordString.c_str());
        in->Insert(&record);
    }
    in->ShutDown();
}

// Checks the number of records filtered through SelectPipe
TEST(RelOpGtests, SelectPipeFilter) {
Pipe *in = new Pipe(100), *out = new Pipe(100);
InsertLeftRecords(in, 20);

CNF cnf;
Record lit;

get_cnf("(left_id < 5)", &left_schema, cnf, lit);

RelOpSelectPipeData data;
data.inPipe = in;
data.outPipe = out;
data.literal = &lit;
data.selOp = &cnf;
SelectPipe selectPipe;
selectPipe.ThreadMethod(&data);

int count = 0;
Record r;
while (out->Remove(&r)) {
count++;
}

EXPECT_EQ(count, 4);
}

TEST(RelOpGtests, JoinWriteToFileTest) {
Pipe *in = new Pipe(100), *out = new Pipe(100);
InsertLeftRecords(in, 20);
in->ShutDown();

Page page;

Join join;
join.writeToFile("abcdef", in);

EXPECT_EQ(fileExists("abcdef"), true);

File file;
file.Open(1, "abcdef");
file.GetPage(&page, 0);

Record r;
int count = 0;
while (page.GetFirst(&r)) {
count++;
}

EXPECT_EQ(count, 20);
file.Close();
remove("abcdef");
}

TEST(RelOpGtests, JoinNestedBlockJoin) {
// Increase out pipe size if increasing left and right
int left = 5, right = 10;
Pipe *l = new Pipe(100), *r = new Pipe(100), *out = new Pipe(100);
InsertLeftRecords(l, left);
InsertRightRecords(r, right);

Join join;
join.NestedBlockJoin(l, r, 10, out);

Record record;
int count = 0;
while(out->Remove(&record)) {
count++;
}

EXPECT_EQ(count, left * right);
}

TEST(RelOpGtests, JoinComparisionBasedJoin) {
// Increase out pipe size if increasing left and right
int left = 50, right = 10;
Pipe *l = new Pipe(100), *r = new Pipe(100), *out = new Pipe(100);
InsertLeftRecords(l, left);
InsertRightRecords(r, right);

OrderMaker oL;
OrderMaker oR;

oL.AddOrder(0, Int);
oR.AddOrder(0, Int);

Join join;
join.ComparisonBasedJoin(l, r, &oL, &oR, out);

int count = 0;
Record record;
while(out->Remove(&record)) {
count++;
}

EXPECT_EQ(count, left < right ? left : right);
}


int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
