#ifndef SYMBOL_TABLE_HEADER
#define SYMBOL_TABLE_HEADER

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct ClockEntry ClockEntry;

struct ClockEntry {
	char * name;
	bool initialized;
	ClockEntry * next;
};

typedef struct {
	ClockEntry * head;
} SymbolTable;

SymbolTable * symbolTableCreate(void);
void          symbolTableDestroy(SymbolTable * table);
bool          symbolTableInsert(SymbolTable * table, const char * name);
ClockEntry *  symbolTableLookup(SymbolTable * table, const char * name);

#endif
