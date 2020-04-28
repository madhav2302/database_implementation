
#ifndef SCHEMA_H
#define SCHEMA_H

#include <stdio.h>
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include <string>

struct att_pair {
	char *name;
	Type type;
};
struct Attribute {

	char *name;
	Type myType;
};

class OrderMaker;
class Schema {

	// gives the attributes in the schema
	int numAtts;
	Attribute *myAtts;

	// gives the physical location of the binary file storing the relation
	char *fileName;

	friend class Record;

public:

	// gets the set of attributes, but be careful with this, since it leads
	// to aliasing!!!
	Attribute *GetAtts ();

	// returns the number of attributes
	int GetNumAtts ();

	// this finds the position of the specified attribute in the schema
	// returns a -1 if the attribute is not present in the schema
	int Find (char *attName);

	// this finds the type of the given attribute
	Type FindType (char *attName);

	// this reads the specification for the schema in from a file
	Schema (char *fName, char *relName);

    Schema(Schema *schema1, Schema *schema2);

	// this composes a schema instance in-memory
	Schema (char *fName, int num_atts, Attribute *atts);

    Schema(Schema *baseSchema, NameList *nameList, int* keepMe);

    Schema(ColumnList *columns);

	// this constructs a sort order structure that can be used to
	// place a lexicographic ordering on the records using this type of schema
	int GetSortOrder (OrderMaker &order);

	char *binaryFileLocation();

    void AliasAttributes(std::string aliasName);

    void WriteToFile(char *tableName);

    void Print();

	~Schema ();

};

#endif
