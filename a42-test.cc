#include <iostream>

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

void test(FuncOperator *finalFunction, TableList *tables, AndList *boolean, NameList *groupingAtts,
          NameList *attsToSelect, int distinctAtts, int distinctFunc) {
    cout << distinctAtts << endl;
}

int main() {
    yyparse();
    test(finalFunction, tables, boolean, groupingAtts, attsToSelect, distinctAtts, distinctFunc);
}

