#include "QueryPlanner.h"

static char *SUM_ATT_NAME = "SUM";
static Attribute doubleAtt = {SUM_ATT_NAME, Double};
static Schema sumSchema("sum_schema", 1, &doubleAtt);

QueryPlanner::QueryPlanner(char *catalog_path, Statistics *statistics, QueryInput *query) {
    this->catalogPath = catalog_path;
    this->statistics = statistics;
    this->queryInput = query;

    MakeQueryPlan();
}

QueryPlanner::~QueryPlanner() = default;

void QueryPlanner::MakeQueryPlan() {
    unordered_map<string, AndList *> tableSelectionAndList;
    vector<AndList *> joins;
    vector<AndList *> joins_arranged;

    SelectTables();
    SplitJoinsAndFilters(&tableSelectionAndList, &joins);
    FilterTables(&tableSelectionAndList);
    MinCostJoins(&joins, &joins_arranged);
    JoinTables(&joins_arranged);
    GroupByTable();
    SumTuples();
    ProjectColumns();
    RemoveDuplicateTuples();
}

void QueryPlanner::SumTuples() {
    // (TODO) Add Distinct support.
    if (queryInput->groupingAtts || !queryInput->finalFunction) return;


    string finalGroupName = GetResultantGroupName();
    RelOpNode *inputRelOpNode = groupToRelOp[finalGroupName];

    auto *function = new Function();
    function->GrowFromParseTree(queryInput->finalFunction, *inputRelOpNode->outputSchema);

    auto *sumNode = new SumRelOpNode();
    sumNode->child1 = inputRelOpNode;
    sumNode->inputPipeId1 = inputRelOpNode->outputPipeId;

    sumNode->outputSchema = &sumSchema;
    sumNode->outputPipeId = nextPipeId++;

    sumNode->computeMe = function;
    sumNode->distinctFunc = queryInput->distinctFunc;

    groupToRelOp[finalGroupName] = sumNode;
}

void QueryPlanner::ProjectColumns() {
    NameList *attsToSelect = queryInput->attsToSelect;

    if (queryInput->finalFunction) {
        auto *sumAtt = new NameList();
        sumAtt->name = SUM_ATT_NAME;
        sumAtt->next = attsToSelect;
        attsToSelect = sumAtt;
    }

    if (!attsToSelect) return;

    string finalGroupName = GetResultantGroupName();
    RelOpNode *inputRelOpNode = groupToRelOp[finalGroupName];

    Schema *inputSchema = inputRelOpNode->outputSchema;

    int *keepMe = nullptr;
    auto *outputSchema = new Schema(inputSchema, attsToSelect, keepMe);

    if (inputSchema->GetNumAtts() == outputSchema->GetNumAtts()) return;

    auto *projectNode = new ProjectRelOpNode();

    projectNode->child1 = inputRelOpNode;
    projectNode->inputPipeId1 = inputRelOpNode->outputPipeId;

    projectNode->outputSchema = outputSchema;
    projectNode->outputPipeId = nextPipeId++;

    projectNode->keepMe = keepMe;
    projectNode->numAttsInput = inputSchema->GetNumAtts();
    projectNode->numAttsOutput = outputSchema->GetNumAtts();

    groupToRelOp[finalGroupName] = projectNode;
}

void QueryPlanner::RemoveDuplicateTuples() {
    if (!queryInput->distinctAtts) return;

    string finalGroupName = GetResultantGroupName();
    RelOpNode *inputRelOpNode = groupToRelOp[finalGroupName];

    auto *duplicateRemovalNode = new DuplicateRemovalRelOpNode();
    duplicateRemovalNode->child1 = inputRelOpNode;
    duplicateRemovalNode->inputPipeId1 = inputRelOpNode->outputPipeId;

    duplicateRemovalNode->outputPipeId = nextPipeId++;
    duplicateRemovalNode->outputSchema = inputRelOpNode->outputSchema;

    duplicateRemovalNode->inputSchema = inputRelOpNode->outputSchema;

    groupToRelOp[finalGroupName] = duplicateRemovalNode;
}

void QueryPlanner::GroupByTable() {
    NameList *nameList = queryInput->groupingAtts;
    if (!nameList) return;

    string finalGroupName = GetResultantGroupName();
    RelOpNode *inputRelOpNode = groupToRelOp[finalGroupName];

    Schema *groupByInputSchema = inputRelOpNode->outputSchema;

    auto *function = new Function();
    function->GrowFromParseTree(queryInput->finalFunction, *groupByInputSchema);

    auto *orderMaker = new OrderMaker(groupByInputSchema, nameList);

    int *keepMe = nullptr;
    auto *groupedAttsSchema = new Schema(groupByInputSchema, nameList, keepMe);

    auto *outputSchema = new Schema(&sumSchema, groupedAttsSchema);

    auto *groupByNode = new GroupByRelOpNode();
    groupByNode->child1 = inputRelOpNode;
    groupByNode->inputPipeId1 = inputRelOpNode->outputPipeId;

    groupByNode->outputSchema = outputSchema;
    groupByNode->outputPipeId = nextPipeId++;

    groupByNode->groupAtts = orderMaker;
    groupByNode->computeMe = function;
    groupByNode->distinctFunc = queryInput->distinctFunc;

    groupToRelOp[finalGroupName] = groupByNode;
}

void
QueryPlanner::SplitJoinsAndFilters(unordered_map<string, AndList *> *tableSelectionAndList, vector<AndList *> *joins) {
    AndList *andList = queryInput->andList;
    while (andList) {
        unordered_map<string, AndList *> currentTableSelectionAndList;

        OrList *orList = andList->left;

        while (orList) {

            Operand *leftOperand = orList->left->left;
            Operand *rightOperand = orList->left->right;

            // Duplicate OrList
            auto *newOrList = new OrList();
            newOrList->left = orList->left;
            if (leftOperand->code != NAME || rightOperand->code != NAME) {
                if (leftOperand->code == NAME || rightOperand->code == NAME) {
                    Operand *nameOperand = leftOperand->code == NAME ? leftOperand : rightOperand;
                    string name = string(nameOperand->value);
                    string relationName = name.substr(0, name.find('.'));

                    if (currentTableSelectionAndList.find(relationName) != currentTableSelectionAndList.end()) {
                        OrList *currentOrList = currentTableSelectionAndList[relationName]->left;
                        while (currentOrList->rightOr) currentOrList = currentOrList->rightOr;
                        currentOrList->rightOr = newOrList;
                    } else {
                        auto *newAndList = new AndList();
                        newAndList->left = newOrList;
                        currentTableSelectionAndList[relationName] = newAndList;
                    }
                }
            } else {
                auto *newAndList = new AndList();
                newAndList->left = newOrList;
                joins->push_back(newAndList);
            }
            orList = orList->rightOr;
        }

        for (auto const &item : currentTableSelectionAndList) {
            if (tableSelectionAndList->find(item.first) != tableSelectionAndList->end()) {
                AndList *currentAndList = tableSelectionAndList->at(item.first);
                while (currentAndList->rightAnd) currentAndList = currentAndList->rightAnd;
                currentAndList->rightAnd = item.second;
            } else {
                (*tableSelectionAndList)[item.first] = item.second;
            }
        }

        andList = andList->rightAnd;
    }
}

void QueryPlanner::FilterTables(unordered_map<string, AndList *> *tableSelectionAndList) {
    for (auto const &item : *tableSelectionAndList) {
        string relName = item.first;
        string groupName = relToGroup[relName];
        RelOpNode *inputRelOpNode = groupToRelOp[groupName];

        AndList *andList = item.second;
        CNF *cnf = new CNF();
        auto *literal = new Record();
        cnf->GrowFromParseTree(andList, inputRelOpNode->outputSchema, *literal); // constructs CNF predicate

        auto *selectPipeNode = new SelectPipeRelOpNode();
        selectPipeNode->child1 = inputRelOpNode;
        selectPipeNode->inputPipeId1 = inputRelOpNode->outputPipeId;

        selectPipeNode->outputSchema = inputRelOpNode->outputSchema;
        selectPipeNode->outputPipeId = nextPipeId++;

        selectPipeNode->selOp = cnf;
        selectPipeNode->literal = literal;

        groupToRelOp[groupName] = selectPipeNode;

        char *applyRelNames[] = {const_cast<char *>(relName.c_str())};
        statistics->Apply(andList, applyRelNames, 1);
    }
}

void QueryPlanner::MinCostJoins(vector<AndList *> *joins, vector<AndList *> *joins_arranged) {
    int numberOfJoins = joins->size();
    if (numberOfJoins < 1) return;

    int initialPermutation[numberOfJoins];
    for (int index = 0; index < numberOfJoins; index++) initialPermutation[index] = index;

    vector<int *> permutations;

    permutationOfJoins(initialPermutation, joins->size(), joins->size(), &permutations);

    int minI = -1;
    double minIntermediateTuples = DBL_MAX;
    for (int outer = 0; outer < permutations.size(); outer++) {
        double permutationIntermediateTuples = 0.0;
        Statistics dummy(*statistics);

        int relNamesIndex = 0;
        char **relNames = new char *[2 * numberOfJoins];
        unordered_set<string> relNamesSet;
        for (int inner = 0; inner < numberOfJoins; inner++) {
            AndList *currentAndList = joins->at(permutations[outer][inner]);
            string attNameWithRelName1 = string(currentAndList->left->left->left->value);
            string attNameWithRelName2 = string(currentAndList->left->left->right->value);

            string relName1 = attNameWithRelName1.substr(0, attNameWithRelName1.find('.'));
            string relName2 = attNameWithRelName2.substr(0, attNameWithRelName2.find('.'));

            if (relNamesSet.find(relName1) == relNamesSet.end()) {
                relNamesSet.insert(string(relName1));
                char *newRel = new char[relName1.length() + 1];
                strcpy(newRel, relName1.c_str());
                relNames[relNamesIndex++] = newRel;
            }

            if (relNamesSet.find(relName2) == relNamesSet.end()) {
                relNamesSet.insert(relName2);
                char *newRel = new char[relName2.length() + 1];
                strcpy(newRel, relName2.c_str());
                relNames[relNamesIndex++] = newRel;
            }

            double intermediate = dummy.Estimate(currentAndList, relNames, relNamesIndex);
            permutationIntermediateTuples += intermediate;
            dummy.Apply(currentAndList, relNames, relNamesIndex);

        }
        if (permutationIntermediateTuples < minIntermediateTuples) {
            minIntermediateTuples = permutationIntermediateTuples;
            minI = outer;
        }
    }

    for (int i = 0; i < numberOfJoins; i++) joins_arranged->push_back(joins->at(permutations[minI][i]));
}

void permutationOfJoins(int *a, int size, int n, vector<int *> *permutations) {
    if (size != 1) {
        for (int index = 0; index < size; index++) {
            permutationOfJoins(a, size - 1, n, permutations);
            swap(a[(size % 2 == 1 ? 0 : index)], a[size - 1]);
        }
    } else {
        int *newPermutation = new int[n];
        for (int index = 0; index < n; index++) newPermutation[index] = a[index];

        permutations->push_back(newPermutation);
    }
}

void QueryPlanner::JoinTables(vector<AndList *> *joins) {
    for (AndList *andList : *joins) {
        OrList *orList = andList->left;

        string leftOperandName = string(orList->left->left->value);
        string tableName1 = leftOperandName.substr(0, leftOperandName.find('.'));
        string groupName1 = relToGroup[tableName1];
        RelOpNode *inputRelOp1 = groupToRelOp[groupName1];

        string rightOperandName = string(orList->left->right->value);
        string tableName2 = rightOperandName.substr(0, rightOperandName.find('.'));
        string groupName2 = relToGroup[tableName2];
        RelOpNode *inputRelOp2 = groupToRelOp[groupName2];

        // constructs CNF predicate
        CNF *cnf = new CNF();
        auto *literal = new Record();
        cnf->GrowFromParseTree(andList, inputRelOp1->outputSchema, inputRelOp2->outputSchema, *literal);

        auto *joinNode = new JoinRelOpNode();
        joinNode->child1 = inputRelOp1;
        joinNode->inputPipeId1 = inputRelOp1->outputPipeId;

        joinNode->child2 = inputRelOp2;
        joinNode->inputPipeId2 = inputRelOp2->outputPipeId;

        auto *outputSchema = new Schema(inputRelOp1->outputSchema, inputRelOp2->outputSchema);
        joinNode->outputSchema = outputSchema;
        joinNode->outputPipeId = nextPipeId++;

        joinNode->selOp = cnf;
        joinNode->literal = literal;

        string newGroupName;
        newGroupName.append(groupName1).append("&").append(groupName2);
        relToGroup[tableName1] = newGroupName;
        relToGroup[tableName2] = newGroupName;

        groupToRelOp.erase(groupName1);
        groupToRelOp.erase(groupName2);
        groupToRelOp[newGroupName] = joinNode;
    }
}

void QueryPlanner::SelectTables() {
    TableList *tableList = queryInput->tables;

    while (tableList) {
        auto *selectFileNode = new SelectFileRelOpNode();

        // Create Schema with aliased attributes.
        auto *schema = new Schema(catalogPath, tableList->tableName);
        schema->AliasAttributes(tableList->aliasAs);
        selectFileNode->outputSchema = schema;

        selectFileNode->outputPipeId = nextPipeId++;

        relToGroup[tableList->aliasAs] = tableList->aliasAs;
        groupToRelOp[tableList->aliasAs] = selectFileNode;

        tableList = tableList->next;
    }
}

string QueryPlanner::GetResultantGroupName() {
    if (groupToRelOp.size() != 1) {
        cerr << "Query is not correct. Group by can be performed on only one group\n";
        exit(1);
    }
    return groupToRelOp.begin()->first;
}

void QueryPlanner::Print() {
    cout << "Print POST order traversal of tree:\n";
    PostOrderQueryPlan(groupToRelOp[GetResultantGroupName()]);
}

void QueryPlanner::PostOrderQueryPlan(RelOpNode *node) {
    if (node == nullptr) return;

    PostOrderQueryPlan(node->child1);
    PostOrderQueryPlan(node->child2);
    node->Print();
} 