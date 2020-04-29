#include "gtest/gtest.h"
#include "Database.h"
#include "ParseTree.h"
#include <fstream>
#include <iostream>

using namespace std;

bool fileExists(std::string fileName) {
    ifstream f(fileName.c_str());
    return f.good();
}


TEST(DBMSTest, CreateTable){
char *tableName = "test_table";
ColumnList column1;
column1.name = "att1";
column1.type = "INTEGER";

ColumnList column2;
column2.name = "att2";
column2.type = "DOUBLE";

ColumnList column3;
column3.name = "att3";
column3.type = "STRING";

column1.next = &column2;
column2.next = &column3;
column3.next = nullptr;
Database db;

db.Create(tableName, &column1, nullptr);

EXPECT_EQ(fileExists("schema/test_table"), true);
EXPECT_EQ(fileExists("data/test_table.bin"), true);
EXPECT_EQ(fileExists("data/test_table.bin.metadata"), true);

remove("schema/test_table");
remove("data/test_table.bin");
remove("data/test_table.bin.metadata");
}

TEST(DBMSTest, DropTable) {
char *tableName = "test_table";
ColumnList column1;
column1.name = "att1";
column1.type = "INTEGER";
column1.next = nullptr;

Database db;
db.Create(tableName, &column1, nullptr);
db.Drop(tableName);

EXPECT_EQ(fileExists("schema/test_table"), false);
EXPECT_EQ(fileExists("data/test_table.bin"), false);
EXPECT_EQ(fileExists("data/test_table.bin.metadata"), false);
}


int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}