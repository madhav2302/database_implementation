#include "gtest/gtest.h"
#include "Database.h"
#include "Schema.h"
#include "ParseTree.h"
#include "QueryPlanner.h"

extern "C" {
    int yyparse(void);   // defined in y.tab.c
    struct YY_BUFFER_STATE *yy_scan_string(const char *);
}

extern struct TableList *tables; // the list of tables and aliases in the query
extern struct ColumnList *myColumns; //
extern struct NameList *orderAtts; //

TEST(DBMSTest, CreateTable){
    Database db;
    char *input = "CREATE TABLE mytable (att1 INTEGER, att2 DOUBLE, att3 STRING) AS HEAP";
    yy_scan_string(input);
    cout<<"Table Name "<<tables->tableName;
    cout<<"Columns "<<myColumns;
    cout<<"OrderAtts"<<orderAtts;
    db.Create(tables->tableName, myColumns, orderAtts);

}

TEST(DBMSTest, DropTable){
    Database db;
    char *input = "DROP TABLE mytable";
    yy_scan_string(input);
    db.Drop(tables->tableName);
}


int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}