#include "gtest/gtest.h"
#include <vector>
#include "QueryPlanner.h"

TEST(QueryPlannerGTests, NumberOfPermutation) {
int numberOfJoins = 10;

int expected = 1;

int initialPermutation[numberOfJoins];
for (int index = 0; index < numberOfJoins; index++)  {
    initialPermutation[index] = index;
    expected *= (index + 1);

}

vector<int *> permutations;

permutationOfJoins(initialPermutation, numberOfJoins, numberOfJoins, &permutations);

EXPECT_EQ(permutations.size(), expected);
}

TEST(QueryPlannerGTests, SelectShouldCreateNNodes) {
TableList *tableList = new TableList();

tableList->tableName = "nation";
tableList->aliasAs = "n";

TableList *tableList1 = new TableList();
tableList1->tableName = "customer";
tableList1->aliasAs = "c";
tableList1->next = tableList;

QueryInput query = {nullptr, tableList1, nullptr, nullptr, nullptr, -1, -1};

QueryPlanner queryPlanner("catalog", nullptr, &query);
queryPlanner.SelectTables();

EXPECT_EQ(queryPlanner.GetGroupToRelOp().size(), 2);
}


int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
