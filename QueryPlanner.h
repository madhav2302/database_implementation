#ifndef DATABASE_IMPLEMENTATION_QUERY_PLANNER_H
#define DATABASE_IMPLEMENTATION_QUERY_PLANNER_H

#include <vector>
#include <cfloat>
#include "ParseTree.h"
#include "Statistics.h"
#include "Schema.h"
#include "Function.h"
#include "QueryPlannerNode.h"
#include "Comparison.h"
#include "RelOp.h"

void permutationOfJoins(int *a, int size, int n, vector<int *> *permutations);

struct QueryInput {
    FuncOperator *finalFunction;
    TableList *tables;
    AndList *andList;
    NameList *groupingAtts;
    NameList *attsToSelect;
    int distinctAtts;
    int distinctFunc;
};


class QueryPlanner {
private:
    char *catalogPath;
    QueryInput *queryInput;
    Statistics *statistics;

    unordered_map<string, RelOpNode *> groupToRelOp;
    unordered_map<string, string> relToGroup;

    int nextPipeId = 1;

    // Split the AndList into selection and joins.
    void SplitJoinsAndFilters(unordered_map<string, AndList *> *tableSelectionAndList, vector<AndList *> *joins);

    void FilterTables(unordered_map<string, AndList *> *tableSelectionAndList);
    void MinCostJoins(vector<AndList *> *joins, vector<AndList *> *joins_arranged);
    void JoinTables(vector<AndList *> *joins);

    // Apply group by if it is in the query
    void GroupByTable();

    // Apply Function if it is in the query and if there is no group by in the query.
    void SumTuples();

    // Apply Duplicate removal.
    void RemoveDuplicateTuples();

    void ProjectColumns();

    static void PostOrderQueryPlan(RelOpNode *node);

    string GetResultantGroupName();

public:
    QueryPlanner(char *catalog_path, Statistics *statistics, QueryInput *query);
    ~QueryPlanner();

    // Load all the tables using SelectFile.
    void SelectTables();

    void Print();

    unordered_map<string, RelOpNode *> GetGroupToRelOp();

private:
    void MakeQueryPlan();
};

#endif