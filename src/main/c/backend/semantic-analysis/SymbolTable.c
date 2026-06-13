#include "SymbolTable.h"

SymbolTable * symbolTableCreate(void) {
	SymbolTable * table = calloc(1, sizeof(SymbolTable));
	return table;
}

void symbolTableDestroy(SymbolTable * table) {
	if (table == NULL) return;
	ClockEntry * current = table->head;
	while (current != NULL) {
		ClockEntry * next = current->next;
		free(current);
		current = next;
	}
	free(table);
}

bool symbolTableInsert(SymbolTable * table, const char * name) {
	if (symbolTableLookup(table, name) != NULL) return false;
	ClockEntry * entry = calloc(1, sizeof(ClockEntry));
	entry->name = (char *) name;
	entry->initialized = true;
	entry->next = table->head;
	table->head = entry;
	return true;
}

ClockEntry * symbolTableLookup(SymbolTable * table, const char * name) {
	ClockEntry * current = table->head;
	while (current != NULL) {
		if (strcmp(current->name, name) == 0) return current;
		current = current->next;
	}
	return NULL;
}
