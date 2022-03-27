/* Implements a dynamically-allocated symbol table */
#ifndef _TABLE_H
#define _TABLE_H

/** ABSOLUTE symbol type */
typedef enum symbol_type {
	CODE_SYMBOL,
	DATA_SYMBOL,
	EXTERNAL_SYMBOL,
	/** Address that contains a reference to (a usage of) external symbol */
	EXTERNAL_REFERENCE,
	ENTRY_SYMBOL
} symbol_type;

/** pointer to table entry is just a table. */
typedef struct entry* table;

/** ABSOLUTE single table entry */
typedef struct entry {
	/** Next entry in table */
	table next;
    /** Key (symbol name) is a string (aka char*) */
    char *key;
	/** Address of the symbol */
	long value;
    /** Base part of the address of the symbol */
    long base;
    /** Offset part of the address of the symbol */
    long offset;
	/** Symbol type */
	symbol_type type;
} table_entry;

/**
 * Adds an item to the table, keeping it sorted.
 * @param tab ABSOLUTE pointer to the table
 * @param key The key of the entry to insert
 * @param value The value of the entry to insert
 * @param type The type of the entry to insert
 */
void add_table_item(table *tab, char *key, long value, symbol_type type);

/**
 * Deallocates all the memory required by the table.
 * @param tab The table to deallocate
 */
void free_table(table tab);

/**
 * Adds the value to add into the value of each entry
 * @param tab The table, containing the entries
 * @param to_add The value to add
 * @param type The type of symbols to add the value to
 */
void update_symbol_table_value(table tab, long to_add, symbol_type type);

/**
 * Returns all the entries by their type in a new table
 * @param tab The table
 * @param type The type to look for
 * @return ABSOLUTE new table, which contains the entries
 */
table filter_table_by_type(table tab, symbol_type type);

/**
 * Find entry from the only specified types
 * @param table_entry The table
 * @param symbol_count The count of given types
 * @param ... Types to include in the lookup
 * @return The entry if found, NULL if not found
 */
table_entry *find_by_types(table table_entry, char *key, int symbol_count, ...);

#endif