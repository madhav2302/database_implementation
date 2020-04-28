#include <iostream>
#include "ParseTree.h"
#include "Database.h"
#include "QueryPlanner.h"

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

int outputOption = 0;
char *outputFile = nullptr;

int clear_pipe(Pipe *in_pipe, Schema *schema) {
    Record rec;
    int cnt = 0;
    while (in_pipe->Remove(&rec)) {
        rec.Print(schema);
        cnt++;
    }
    return cnt;
}

void test() {
    Database database;
    if (operationCode == CREATE_OP) {
        database.Create(tables->tableName, myColumns, orderAtts);
    } else if (operationCode == INSERT_OP) {
        database.Load(tables->tableName, fileName);
    } else if (operationCode == DROP_OP) {
        database.Drop(tables->tableName);
    } else if (operationCode == SET_OP) {
        if (strcmp(fileName, "STDOUT") == 0) {
            outputOption = 0;
        } else if (strcmp(fileName, "NONE") == 0) {
            outputOption = 1;
        } else {
            outputOption = 2;
            outputFile = fileName;
        }
    } else if (operationCode == SELECT_OP) {
        cout << '\n';
        Statistics statistics;
        statistics.Read("Statistics-42.txt");

        QueryInput query = {finalFunction, tables, boolean, groupingAtts, attsToSelect, distinctAtts, distinctFunc};
        QueryPlanner queryPlanner("schema/", &statistics, &query);

        if (outputOption == 1) {
            cout << "Query Plan\n";
            queryPlanner.Print();

        } else if (outputOption == 2) {
            FILE *writefile = fopen(outputFile, "w");
            RelOpNode *relOpNode = queryPlanner.Execute();
            WriteOut writeOut;
            writeOut.Run(*relOpNode->pipe, writefile, *relOpNode->outputSchema);
            queryPlanner.CleanUp();
            writeOut.WaitUntilDone();

            cout << "Output is written to : " << outputFile << '\n';
        } else {
            cout << "Query Result\n";
            RelOpNode *relOpNode = queryPlanner.Execute();
            clear_pipe(relOpNode->pipe, relOpNode->outputSchema);
            queryPlanner.CleanUp();
        }
        cout << "\n\n\n";
    } else {
        cerr << "Unknown operation code : " << operationCode << '\n';
        exit(1);
    }
}

// CREATE TABLE nation (n_nationkey INTEGER, n_name STRING, n_regionkey INTEGER, n_comment STRING) AS HEAP
// CREATE TABLE region (r_regionkey INTEGER, r_name STRING, r_comment STRING) AS HEAP
// CREATE TABLE customer (c_custkey INTEGER, c_name STRING, c_address STRING, c_nationkey INTEGER, c_phone STRING, c_acctbal DOUBLE, c_mktsegment STRING, c_comment STRING) AS HEAP
// CREATE TABLE supplier (s_suppkey INTEGER, s_name STRING, s_address STRING, s_nationkey INTEGER, s_phone STRING, s_acctbal DOUBLE, s_comment STRING) AS HEAP
// CREATE TABLE partsupp (ps_partkey INTEGER, ps_suppkey INTEGER, ps_availqty INTEGER, ps_supplycost DOUBLE, ps_comment STRING) AS HEAP
// CREATE TABLE part (p_partkey INTEGER, p_name STRING, p_mfgr STRING, p_brand STRING, p_type STRING, p_size INTEGER, p_container STRING, p_retailprice DOUBLE, p_comment STRING) AS HEAP

// INSERT '/Users/madhavsodhani/projects/cpp/mock_data/nation.tbl' INTO nation
// INSERT '/Users/madhavsodhani/projects/cpp/mock_data/region.tbl' INTO region
// INSERT '/Users/madhavsodhani/projects/cpp/mock_data/customer.tbl' INTO customer
// INSERT '/Users/madhavsodhani/projects/cpp/mock_data/supplier.tbl' INTO supplier
// INSERT '/Users/madhavsodhani/projects/cpp/mock_data/part.tbl' INTO part
// INSERT '/Users/madhavsodhani/projects/cpp/mock_data/partsupp.tbl' INTO partsupp

// ** STORM **
// INSERT '/cise/homes/msodhani/tpch-dbgen/nation.tbl' INTO nation
// INSERT '/cise/homes/msodhani/tpch-dbgen/region.tbl' INTO region
// INSERT '/cise/homes/msodhani/tpch-dbgen/customer.tbl' INTO customer
// INSERT '/cise/homes/msodhani/tpch-dbgen/supplier.tbl' INTO supplier
// INSERT '/cise/homes/msodhani/tpch-dbgen/part.tbl' INTO part
// INSERT '/cise/homes/msodhani/tpch-dbgen/partsupp.tbl' INTO partsupp

int main() {
    while (true) {
        cout << "Select option : \n";
        cout << "1. Execute Query \n";
        cout << "2. Exit \n";

        int option = 0;
        cin >> option;

        if (option != 1 && option != 2) {
            cout << "Unknown option : " << option << '\n';

            continue;
        }

        if (option == 1) {
            cout << "\nEnter Query \n";
            if (yyparse() != 0) {
                cerr << "Some problem with yyparse\n";
                exit(1);
            }

            test();
        } else {
            break;
        }
    }
}


