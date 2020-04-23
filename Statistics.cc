#include "Statistics.h"

Statistics::Statistics() {
}

Statistics::Statistics(Statistics &copyMe) {
    for (auto &x : copyMe.groupToRelation) {
        groupToRelation[x.first] = StatisticsRelation(x.second.GetNumOfTuples());
    }

    for (auto &attNameToAttributeMapItem : copyMe.nameToAttribute) {
        nameToAttribute[attNameToAttributeMapItem.first] =
                StatisticsAttribute(attNameToAttributeMapItem.second.GetNumOfDistinct());
    }

    for (auto &setNameToSetOfRelationsMapItem : copyMe.groupNameToRelations) {
        unordered_set<string> newRelationSet;
        for (auto &relName : setNameToSetOfRelationsMapItem.second) {
            newRelationSet.insert(relName);
        }
        groupNameToRelations[setNameToSetOfRelationsMapItem.first] = newRelationSet;
    }

    for (auto &relNameToSetNameMapItem : copyMe.relToGroup) {
        relToGroup[relNameToSetNameMapItem.first] = relNameToSetNameMapItem.second;
    }
}

Statistics::~Statistics() {
}

void Statistics::AddRel(char *relName, int numTuples) {
    if (!(groupNameToRelations.find(relName) == groupNameToRelations.end())) {
        if (relToGroup[relName] == relName) {
            groupToRelation[relName].SetNumOfTuples(numTuples);

            // Otherwise throw an error, as table is already joined.
        } else {
            cerr << "Relation is already joined with some table.\n";
            exit(1);
        }
    } else {
        unordered_set<string> newRelationSet;
        newRelationSet.insert(relName);

        relToGroup[relName] = relName;
        groupNameToRelations[relName] = newRelationSet;
        groupToRelation[relName] = StatisticsRelation(numTuples);
    }
}

void Statistics::AddAtt(const string& relName, string attName, int numDistincts) {
    string attNameWithRelName = relName + "." + attName;

    if (nameToAttribute.find(attNameWithRelName) != nameToAttribute.end()
        && relToGroup[relName] != relName) {
        cerr << "Relation is already joined with some table. Hence attribute can't be updated.\n";
        exit(1);
    }

    if (numDistincts == -1) numDistincts = groupToRelation[relToGroup[relName]].GetNumOfTuples();

    nameToAttribute[attNameWithRelName] = StatisticsAttribute(numDistincts);
}

void Statistics::CopyRel(char *oldName, char *newName) {

    // Add new relation.
    AddRel(newName, groupToRelation[relToGroup[oldName]].GetNumOfTuples());

    // Add attributes in the new relation.
    for (auto attNameToAttributeMapItem : nameToAttribute) {
        string attNameWithRelName = attNameToAttributeMapItem.first;
        string relName = attNameWithRelName.substr(0, attNameWithRelName.find('.'));

        if (relName == string(oldName)) {
            string attName = attNameWithRelName.substr(attNameWithRelName.find('.') + 1);
            AddAtt(string(newName), attName, attNameToAttributeMapItem.second.GetNumOfDistinct());
        }
    }
}

void Statistics::Read(char *fromWhere) {
    ifstream fIn;
    fIn.open(fromWhere);

    if (!fIn.is_open()) return;

    string readLine;

    getline(fIn, readLine);
    getline(fIn, readLine);
    int setNameToRelationMapSize = stoi(readLine);
    groupToRelation.clear();
    for (int i = 0; i < setNameToRelationMapSize; i++) {
        getline(fIn, readLine, '=');
        string groupName = readLine;
        getline(fIn, readLine);
        int numOfTuples = stoi(readLine);
        groupToRelation[groupName] = StatisticsRelation(numOfTuples);
    }

    getline(fIn, readLine);
    getline(fIn, readLine);
    int attNameToAttributeMapSize = stoi(readLine);
    nameToAttribute.clear();
    for (int i = 0; i < attNameToAttributeMapSize; i++) {
        getline(fIn, readLine, '=');
        string attName = readLine;
        getline(fIn, readLine);
        int numOfDistinct = stoi(readLine);
        nameToAttribute[attName] = StatisticsAttribute(numOfDistinct);
    }

    getline(fIn, readLine);
    getline(fIn, readLine);
    int setNameToSetOfRelationsMapSize = stoi(readLine);
    groupNameToRelations.clear();
    for (int i = 0; i < setNameToSetOfRelationsMapSize; i++) {
        getline(fIn, readLine, '=');
        string groupName = readLine;

        unordered_set<string> newRelationSet;
        groupNameToRelations[groupName] = newRelationSet;

        getline(fIn, readLine);
        stringstream s_stream(readLine);

        while (s_stream.good()) {
            getline(s_stream, readLine, ',');
            groupNameToRelations[groupName].insert(readLine);
        }
    }

    getline(fIn, readLine);
    getline(fIn, readLine);
    int relNameToSetNameMapSize = stoi(readLine);
    relToGroup.clear();
    for (int i = 0; i < attNameToAttributeMapSize; i++) {
        getline(fIn, readLine, '=');
        string relName = readLine;
        getline(fIn, readLine);
        string groupName = readLine;
        relToGroup[relName] = groupName;
    }

}

void Statistics::Write(char *fromWhere) {
    ofstream fOut;
    fOut.open(fromWhere);

    fOut << "**************** Group Relations *****************\n";
    fOut << groupToRelation.size() << "\n";
    for (auto &x: groupToRelation) {
        fOut << x.first << "=" << x.second.GetNumOfTuples() << "\n";
    }

    fOut << "**************** Attributes ******************\n";
    fOut << nameToAttribute.size() << "\n";
    for (auto &x: nameToAttribute) {
        fOut << x.first << "=" << x.second.GetNumOfDistinct() << "\n";
    }

    fOut << "****************** GroupName to Relations ****************\n";
    fOut << groupNameToRelations.size() << "\n";
    for (auto &x: groupNameToRelations) {
        auto secondIterator = x.second.begin();
        fOut << x.first << "=" << *(secondIterator);
        while (++secondIterator != x.second.end()) {
            fOut << "," << *(secondIterator);
        }
        fOut << "\n";
    }

    fOut << "******************** Relation Name to Group Name *****************\n";
    fOut << relToGroup.size() << "\n";
    for (auto &x: relToGroup) {
        fOut << x.first << "=" << x.second << "\n";
    }
}

void Statistics::Apply(struct AndList *parseTree, char *relNames[], int numToJoin) {
    unordered_set<string> relNamesSet;
    for (int i = 0; i < numToJoin; i++) {
        relNamesSet.insert(relNames[i]);
    }

    ApplyPreProcess(parseTree, &relNamesSet);
    string resultantGroupName;
    unordered_map<string, double> attNameToProbabilitiesMap;
    while (parseTree) {
        attNameToProbabilitiesMap.clear();

        OrList *orList = parseTree->left;
        while (orList) {
            ComparisonOp *currentComparisonOp = orList->left;
            Operand *leftOperand = currentComparisonOp->left;
            Operand *rightOperand = currentComparisonOp->right;
            int comparisonOperator = currentComparisonOp->code;

            // if both side of a operator, there is a name, then Join the two tables.
            if (leftOperand->code != NAME || rightOperand->code != NAME) {
                if (leftOperand->code == NAME ^ rightOperand->code == NAME) {
                    Operand *nameOperand = leftOperand->code == NAME ? leftOperand : rightOperand;
                    string attNameWithRelName = string(nameOperand->value);
                    string relName = attNameWithRelName.substr(0, attNameWithRelName.find('.'));
                    if (currentComparisonOp->code == EQUALS) {
                        double probabilityFraction = 1.0 / nameToAttribute[attNameWithRelName].GetNumOfDistinct();
                        if (attNameToProbabilitiesMap.find(attNameWithRelName) == attNameToProbabilitiesMap.end()) {
                            attNameToProbabilitiesMap[attNameWithRelName] = probabilityFraction;
                        } else {
                            attNameToProbabilitiesMap[attNameWithRelName] += probabilityFraction;
                        }
                    } else {
                        if (attNameToProbabilitiesMap.find(attNameWithRelName) == attNameToProbabilitiesMap.end()) {
                            attNameToProbabilitiesMap[attNameWithRelName] = (1.0 / 3.0);
                        } else {

                        }
                    }
                    resultantGroupName = relToGroup[relName];
                } else {
                    cerr << "left operand " << string(leftOperand->value) << " and right operand "
                         << string(rightOperand->value) << " are not valid.\n";
                    exit(1);
                }
            }
                // Otherwise it is a select operation.
            else {
                if (comparisonOperator != EQUALS) {
                    cerr << "Join is not implemented for other than Equals operator\n";
                    exit(1);
                }

                string leftAttNameWithRelName = string(leftOperand->value);
                int numOfDistinctInLeftAtt = nameToAttribute[leftAttNameWithRelName].GetNumOfDistinct();
                string leftRelName = leftAttNameWithRelName.substr(0, leftAttNameWithRelName.find('.'));
                string leftGroupName = relToGroup[leftRelName];
                double numOfTuplesInLeftGroup = groupToRelation[leftGroupName].GetNumOfTuples();

                string rightAttNameWithRelName = string(rightOperand->value);
                int numOfDistinctInRightAtt = nameToAttribute[rightAttNameWithRelName].GetNumOfDistinct();
                string rightRelName = rightAttNameWithRelName.substr(0, rightAttNameWithRelName.find('.'));
                string rightGroupName = relToGroup[rightRelName];
                double numOfTuplesInRightGroup = groupToRelation[rightGroupName].GetNumOfTuples();

                if (leftGroupName == rightGroupName) {
                    cerr << "Table " << leftRelName << " is already joined with " << rightGroupName << ".\n";
                    exit(1);
                }

                double numOfTuplesPerAttValueInLeft = (numOfTuplesInLeftGroup / numOfDistinctInLeftAtt);
                double numOfTuplesPerAttValueInRight = (numOfTuplesInRightGroup / numOfDistinctInRightAtt);

                double numOfTuplesAfterJoin = numOfTuplesPerAttValueInLeft
                                              * numOfTuplesPerAttValueInRight
                                              * min(numOfDistinctInLeftAtt, numOfDistinctInRightAtt);

                string newGroupName;
                newGroupName.append(leftGroupName).append("&").append(rightGroupName);

                // Delete leftGroups and rightGroups for Different map.
                groupToRelation.erase(leftGroupName);
                groupToRelation.erase(rightGroupName);

                // Create new group relation.
                groupToRelation[newGroupName] = numOfTuplesAfterJoin;
                unordered_set<string> newRelationSet;


                // Change groups of leftGroups and rightGroups relations.
                for (const string &relName : groupNameToRelations[leftGroupName]) {
                    relToGroup[relName] = newGroupName;
                    newRelationSet.insert(relName);
                }
                groupNameToRelations.erase(leftGroupName);

                for (const string &relName : groupNameToRelations[rightGroupName]) {
                    relToGroup[relName] = newGroupName;
                    newRelationSet.insert(relName);
                }
                groupNameToRelations.erase(rightGroupName);

                groupNameToRelations[newGroupName] = newRelationSet;
                resultantGroupName = newGroupName;
            }
            orList = orList->rightOr;
        }

        if (!attNameToProbabilitiesMap.empty()) {
            double numOfTuples = groupToRelation[resultantGroupName].GetNumOfTuples();
            double multiplicationFactor = 0.0;

            if (attNameToProbabilitiesMap.size() == 1) {
                multiplicationFactor = (*attNameToProbabilitiesMap.begin()).second;
            } else {
                double additionFactor = 0.0;
                double subtractionFactor = 1.0;

                for (const auto &attNameToProbabilitiesMapItem : attNameToProbabilitiesMap) {
                    additionFactor += attNameToProbabilitiesMapItem.second;
                    subtractionFactor *= attNameToProbabilitiesMapItem.second;
                }
                multiplicationFactor = additionFactor - subtractionFactor;

            }

            numOfTuples *= multiplicationFactor;


            groupToRelation[resultantGroupName].SetNumOfTuples(numOfTuples);
        }
        parseTree = parseTree->rightAnd;
    }

}

double Statistics::Estimate(struct AndList *parseTree, char **relNames, int numToJoin) {
    Statistics temp(*this);

    temp.Apply(parseTree, relNames, numToJoin);
    unordered_set<string> groups;
    for (int i = 0; i < numToJoin; i++) groups.insert(temp.relToGroup[relNames[i]]);

    if (groups.size() != 1) {
        cerr << "Error while estimating.\n";
        exit(1);
    }

    return temp.groupToRelation[*groups.begin()].GetNumOfTuples();
}

void Statistics::ApplyPreProcess(struct AndList *parseTree, unordered_set<string> *relations) {
    // Validate Relations
    ValidateApplyOnRelations(relations);
    // Check attributes
    ApplyOnAttributesPreProcess(parseTree, relations);
}

void Statistics::ValidateApplyOnRelations(unordered_set<string> *relations) {
    unordered_set<string> setNamesToJoin;
    for (const string &relName : *relations) {
        if (relToGroup.find(relName) == relToGroup.end()) {
            cerr << "Relation " << relName << " is not present in statistics.\n";
            exit(1);
        }
        setNamesToJoin.insert(relToGroup[relName]);
    }

    unordered_set<string> relationsInResult;
    for (const string &setName : setNamesToJoin) {
        for (const string &relName : groupNameToRelations[setName]) relationsInResult.insert(relName);

    }

    for (const string &relName : *relations) relationsInResult.erase(relName);


    if (!relationsInResult.empty()) {
        cerr << "Relation association doesn't make sense\n";
        exit(1);
    }
}

void Statistics::ApplyOnAttributesPreProcess(struct AndList *parseTree, unordered_set<string> *relations) {
    while (parseTree) {
        OrList *orList = parseTree->left;
        while (orList) {
            if (orList->left->left->code == NAME) {
                NameOperandPreProcess(orList->left->left, relations);
            }
            if (orList->left->right->code == NAME) {
                NameOperandPreProcess(orList->left->right, relations);
            }
            orList = orList->rightOr;
        }
        parseTree = parseTree->rightAnd;
    }
}

void Statistics::NameOperandPreProcess(Operand *operand, unordered_set<string> *relations) {
    string operandValue = operand->value;

    if (operandValue.find('.') == string::npos) {
        bool found = false;
        for (const string &rel : *relations) {
            string attributeWithRel = rel + "." + string(operandValue);
            if (nameToAttribute.find(attributeWithRel) != nameToAttribute.end()) {
                found = true;
                char *newOperandValue = new char[attributeWithRel.size() + 1];
                strcpy(newOperandValue, attributeWithRel.c_str());
                operand->value = newOperandValue;
                break;
            }
        }
        if (!found) {
            cerr << "No relation contains attribute " << operandValue << ".\n";
            exit(1);
        }
    } else {
        string relationName = operandValue.substr(0, operandValue.find('.'));
        if (nameToAttribute.find(operandValue) == nameToAttribute.end()) {
            cerr << "Attribute " << string(operandValue) << " is not present in Statistics.\n";
        }
        if (relations->find(relationName) == relations->end()) {
            cerr << "Attribute is not linked with any rel names given.\n";
        }
    }
}