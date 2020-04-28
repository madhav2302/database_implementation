#include "Database.h"
#include "Schema.h"
#include "DBFile.h"
#include "Comparison.h"
#include <iostream>

using namespace std;

void Database::Create(char *tableName, ColumnList *columns, NameList *order) {
    Schema schema(columns);
    schema.WriteToFile(tableName);

    std::string fileName = std::string("data/") + tableName + ".bin";

    if (order) {
        OrderMaker orderMaker(&schema, order);
        SortInfo sortInfo(&orderMaker, 100);
        DBFile dbFile;
        dbFile.Create(fileName.c_str(), sorted, &sortInfo);
        dbFile.Close();
    } else {
        DBFile dbFile;
        dbFile.Create(fileName.c_str(), heap, nullptr);
        dbFile.Close();
    }
}

void Database::Load(char *tableName, char *filePath) {
    char *schemaPath = const_cast<char *>((std::string("schema/") + tableName).c_str());
    Schema schema(schemaPath, tableName);

    DBFile dbFile;

    std::string table = std::string("data/") + tableName + ".bin";

    std::cerr << "Schema Complete\n";
    dbFile.Open(table.c_str());
    dbFile.Load(schema, filePath);
    dbFile.Close();
}

void Database::Drop(char *tableName) {
    std::string schemaPath = std::string("schema/") + tableName;
    remove(schemaPath.c_str());

    std::string table = std::string("data/") + tableName + ".bin";
    remove(table.c_str());

    std::string tableMetadata = std::string("data/") + tableName + ".bin.metadata";
    remove(tableMetadata.c_str());
}
