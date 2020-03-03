#include "gtest/gtest.h"
#include <fstream>
#include <iostream>
#include "SortedDBFile.h"

bool fileExists(std::string fileName) {
    ifstream f(fileName.c_str());
    return f.good();
}

TEST(SortedDBFileGTests, CreateMetadata) {
remove("nation.bin");
remove("nation.bin.metadata");

OrderMaker o;

SortInfo sortInfo(&o, 10);

SortedDBFile file;
file.Create("nation.bin", sorted, &sortInfo);

file.Close();

EXPECT_EQ(fileExists("nation.bin"), true);
EXPECT_EQ(fileExists("nation.bin.metadata"), true);
}

TEST(SortedDBFileGTests, ReadMetadata) {
remove("nation.bin");
remove("nation.bin.metadata");

OrderMaker o;

int runLen = 10;
SortInfo sortInfo(&o, runLen);

SortedDBFile file;
file.Create("nation.bin", sorted, &sortInfo);

EXPECT_EQ(fileExists("nation.bin"), true);
EXPECT_EQ(fileExists("nation.bin.metadata"), true);

ifstream metadata_read("nation.bin.metadata");
if (metadata_read.is_open()) {
    string line;
    getline(metadata_read, line);

    fType type = (fType) std::stoi(line);
    EXPECT_EQ(type, sorted);

    getline(metadata_read, line);
    int runLenInMetadata = std::stoi(line);

    EXPECT_EQ(runLenInMetadata, runLen);

    getline(metadata_read, line);
    int numAtts = std::stoi(line);
    EXPECT_EQ(numAtts, 0);

    metadata_read.close();
}
}



int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}