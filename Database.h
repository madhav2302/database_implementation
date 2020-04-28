# include "ParseTree.h"

#ifndef DATABASE_IMPLEMENTATION_DATABASE_H
#define DATABASE_IMPLEMENTATION_DATABASE_H


class Database {


public:
    void Create(char *tableName, ColumnList *columns, NameList *order);

    void Load(char *tableName, char *fileName);

    void Drop(char *tableName);
};


#endif //DATABASE_IMPLEMENTATION_DATABASE_H
