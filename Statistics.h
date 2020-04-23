#ifndef STATISTICS_
#define STATISTICS_

#include "ParseTree.h"

#include <cstring>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include<sstream>

using namespace std;

struct StatisticsRelation {
private:
    double numberOfTuples;

public:
    StatisticsRelation() {
        this->numberOfTuples = 0;
    }

    StatisticsRelation(double numOfTuples) {
        this->numberOfTuples = numOfTuples;
    }

    double GetNumOfTuples() {
        return numberOfTuples;
    }

    void SetNumOfTuples(double n) {
        this->numberOfTuples = n;
    }
};

struct StatisticsAttribute {
private:
    int numberOfDistinct;

public:
    StatisticsAttribute() {
        this->numberOfDistinct = 0;
    }

    StatisticsAttribute(int numOfDistinct) {
        this->numberOfDistinct = numOfDistinct;
    }

    int GetNumOfDistinct() {
        return numberOfDistinct;
    }
};

class Statistics {

private:
    unordered_map<string, StatisticsRelation> groupToRelation;
    unordered_map<string, StatisticsAttribute> nameToAttribute;
    unordered_map<string, unordered_set<string> > groupNameToRelations;
    unordered_map<string, string> relToGroup;

    void ValidateApplyOnRelations(unordered_set<string> *relations);
    void ApplyPreProcess(struct AndList *parseTree, unordered_set<string> *relations);
    void ApplyOnAttributesPreProcess(struct AndList *parseTree, unordered_set<string> *relations);
    void NameOperandPreProcess(Operand *operand, unordered_set<string> *relations);

public:
    Statistics();

    Statistics(Statistics &copyMe);     // Performs deep copy

    ~Statistics();

    void AddRel(char *relName, int numTuples);

    void AddAtt(const string& relName, string attName, int numDistincts);

    void CopyRel(char *oldName, char *newName);

    void Read(char *fromWhere);

    void Write(char *fromWhere);

    void Apply(struct AndList *parseTree, char *relNames[], int numToJoin);

    double Estimate(struct AndList *parseTree, char **relNames, int numToJoin);
};

#endif