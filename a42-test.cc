#include <iostream>
#include "ParseTree.h"
#include "QueryPlanner.h"
#include "Statistics.h"

using namespace std;

extern "C" {
int yyparse(void);   // defined in y.tab.c
}

extern struct FuncOperator *finalFunction;
extern struct TableList *tables;
extern struct AndList *boolean;
extern struct NameList *groupingAtts;
extern struct NameList *attsToSelect;
extern int distinctAtts;
extern int distinctFunc;

// Load table data
void createStatisticsForTest() {
    Statistics s;
    char *relName[] = {"region", "nation", "part", "supplier", "partsupp", "customer", "orders", "lineitem"};

    // Add region relation
    s.AddRel(relName[0],5);
    // Add attributes to region relation
    s.AddAtt(relName[0], "r_regionkey",5);
    s.AddAtt(relName[0], "r_name",5);
    // Copy nation relation
    s.CopyRel("region","r");

    // Add nation relation
    s.AddRel(relName[1],25);
    // Add attributes to nation relation
    s.AddAtt(relName[1], "n_nationkey",25);
    s.AddAtt(relName[1], "n_regionkey",5);
    s.AddAtt(relName[1], "n_name",25);
    // Copy nation relation
    s.CopyRel("nation","n");
    s.CopyRel("nation","n1");
    s.CopyRel("nation","n2");

    // Add part relation
    s.AddRel(relName[2],200000);
    // Add attributes to part
    s.AddAtt(relName[2], "p_partkey",200000);
    s.AddAtt(relName[2], "p_name", 199996);
    s.AddAtt(relName[2], "p_size",50);
    s.AddAtt(relName[2], "p_container",40);
    // Copy part relation
    s.CopyRel("part","p");

    // Add supplier relation
    s.AddRel(relName[3],10000);
    // Add attributes to supplier
    s.AddAtt(relName[3], "s_suppkey",10000);
    s.AddAtt(relName[3], "s_nationkey",25);
    s.AddAtt(relName[3], "s_acctbal",9955);
    // Copy supplier relation
    s.CopyRel("supplier","s");

    // Add partsupp relation
    s.AddRel(relName[4],800000);
    // Add attributes to partsupp
    s.AddAtt(relName[4], "ps_suppkey", 10000);
    s.AddAtt(relName[4], "ps_partkey", 200000);
    s.AddAtt(relName[4], "ps_supplycost", 99865);
    // Copy partsupp relation
    s.CopyRel("partsupp","ps");

    // Add customer relation
    s.AddRel(relName[5],150000);
    // Add attributes to customer
    s.AddAtt(relName[5], "c_custkey",150000);
    s.AddAtt(relName[5], "c_nationkey",25);
    s.AddAtt(relName[5], "c_mktsegment",5);
    // Copy customer relation
    s.CopyRel("customer","c");

    // Add orders relation
    s.AddRel(relName[6],1500000);
    // Add attributes to orders
    s.AddAtt(relName[6], "o_orderkey",1500000);
    s.AddAtt(relName[6], "o_custkey",150000);
    s.AddAtt(relName[6], "o_orderdate",99996);

    // Add lineitem relation
    s.AddRel(relName[7],6001215);
    // Add attributes to lineitem
    s.AddAtt(relName[7], "l_partkey",200000);
    s.AddAtt(relName[7], "l_shipinstruct",4);
    s.AddAtt(relName[7], "l_returnflag",3);
    s.AddAtt(relName[7], "l_discount",11);
    s.AddAtt(relName[7], "l_shipmode",7);
    s.AddAtt(relName[7], "l_orderkey",1500000);
    s.AddAtt(relName[7], "l_receiptdate",198455);

    s.Write("Statistics-42.txt");
}


int main() {
    // Loading Statistics from the file.
    Statistics statistics;
    statistics.Read("Statistics-42.txt");

    // Parse the query.
    if (yyparse() != 0) {
        cout << "Can't parse your SQL.\n";
        exit(1);
    }

    QueryInput query = {finalFunction, tables, boolean, groupingAtts, attsToSelect, distinctAtts, distinctFunc};

    // Create query plan/
    QueryPlanner queryPlan("catalog", &statistics, &query);

    // Print the query plan in post order.
    queryPlan.Print();
}
