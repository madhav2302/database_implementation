#include <iostream>
#include "ParseTree.h"

using namespace std;

extern "C" {
int yyparse(void);   // defined in y.tab.c
}

extern struct FuncOperator *finalFunction; // the aggregate function (NULL if no agg)
extern struct TableList *tables; // the list of tables and aliases in the query
extern struct AndList *boolean; // the predicate in the WHERE clause
extern struct NameList *groupingAtts; // grouping atts (NULL if no grouping)
extern struct NameList *attsToSelect; // the set of attributes in the SELECT (NULL if no such atts)
extern int distinctAtts; // 1 if there is a DISTINCT in a non-aggregate query
extern int distinctFunc;  // 1 if there is a DISTINCT in an aggregate query
extern struct ColumnList *myColumns; //
extern struct NameList *orderAtts; //
extern int operationCode;
extern char *fileName;

void test() {
    if (operationCode == CREATE_OP) {
        cout << "CREATE Operation\n";
        cout << "Table Name : " << tables->tableName << '\n';

        ColumnList *columnList = myColumns;

        while (columnList) {
            cout << "Column Name : " << columnList->name << ", Type : " << columnList->type << '\n';
            columnList = columnList->next;
        }

        if (orderAtts) {
            cout << "\nSorted File Order\n";

            NameList *nameList = orderAtts;
            while (nameList) {
                cout << nameList->name << '\n';
                nameList = nameList->next;
            }
        } else {
            cout << "Heap File\n";
        }
    } else if (operationCode == INSERT_OP) {
        cout << "INSERT Operation\n";

        cout << "File Name : " << fileName << '\n';
        cout << "Table Name : " << tables->tableName << '\n';
    } else if (operationCode == DROP_OP) {
        cout << "DROP Operation\n";
        cout << "Table Name : " << tables->tableName << '\n';
    } else if (operationCode == SET_OP) {
        cout << "SET OUTPUT Operation\n";
        cout << "Type or FileName : " << fileName << '\n';
    } else if (operationCode == SELECT_OP) {
        cout << "SELECT Operation\n";
    } else {
        cerr << "Unknown operation code : " << operationCode << '\n';
        exit(1);
    }
}

// CREATE TABLE mytable (att1 INTEGER, att2 DOUBLE, att3 STRING) AS HEAP
// CREATE TABLE MYTABLE (att1 INTEGER, att2 DOUBLE, att3 STRING) AS SORTED ON att1, att2
// INSERT output.txt INTO mytable
int main() {
    if (yyparse() != 0) {
        cerr << "Some problem with yyparse\n";
        exit(1);
    }

    test();
}


