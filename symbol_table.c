#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "symbol_table.h"
#include "helper.h"

/* Table data structure based on sorted linked list */

void add_table_item(table *tab, char *key, long value, symbol_type type) {
	char *temp_key;
	table prev_entry, curr_entry, new_entry;
    long offset = value % 16; /* Calc offset as explained in direct addressing */
	/* allocate memory for new entry */
	new_entry = (table) better_malloc(sizeof(table_entry));
    /* +1 == '/0' */
	temp_key = (char *) better_malloc(strlen(key) + 1);
	strcpy(temp_key, key);
	new_entry->key = temp_key;
	new_entry->value = value;
	new_entry->type = type;
    new_entry->base = value - offset;
    new_entry->offset = offset;
	/* if the table's null, set the new entry as the head. */
	if ((*tab) == NULL || (*tab)->value > value) {
		new_entry->next = (*tab);
		(*tab) = new_entry;
		return;
	}

	/* Insert the new table entry, keeping it sorted */
	curr_entry = (*tab)->next;
	prev_entry = *tab;
	while (curr_entry != NULL && curr_entry->value < value) {
		prev_entry = curr_entry;
		curr_entry = curr_entry->next;
	}

	new_entry->next = curr_entry;
	prev_entry->next = new_entry;
}

void free_table(table tab) {
	table prev_entry, curr_entry = tab;
	while (curr_entry != NULL) {
		prev_entry = curr_entry;
		curr_entry = curr_entry->next;
		free(prev_entry->key); /* Didn't forget you!ssss */
		free(prev_entry);
	}
}

void update_symbol_table_value(table tab, long to_add, symbol_type type) {
	table curr_entry;

	for (curr_entry = tab; curr_entry != NULL; curr_entry = curr_entry->next) {
        /* update only values with the specific symbol_type */
		if (curr_entry->type == type) {
            int new_value, new_offset;
            new_value = curr_entry->value + to_add;
            new_offset = new_value % 16;
            curr_entry->value = new_value;
            curr_entry->base = new_value - new_offset;
            curr_entry->offset = new_offset;

		}
	}
}

table filter_table_by_type(table tab, symbol_type type) {
	table new_table = NULL;
	/* Insert to the new table entries with the corresponding type */
	do {
		if (tab->type == type) {
			add_table_item(&new_table, tab->key, tab->value, tab->type);
		}
	} while ((tab = tab->next) != NULL);
	return new_table; /* It holds a pointer to the first entry, dynamically-allocated, so it's fine (not in stack) */
}

table_entry *find_by_types(table table_entry, char *key, int symbol_count, ...) {
	int i;
    /* table null => return */
    if (table_entry == NULL) {
        return NULL;
    }
	symbol_type *valid_symbol_types = better_malloc((symbol_count) * sizeof(int));
	/* Build a list of the valid types */
	va_list arg_list;
	va_start(arg_list, symbol_count);
	for (i = 0; i < symbol_count; i++) {
		valid_symbol_types[i] = va_arg(arg_list, symbol_type);
	}
	va_end(arg_list);

	/* Iterate over the table and return the table_entry if found */
	do {
		for (i = 0; i < symbol_count; i++) {
			if (valid_symbol_types[i] == table_entry->type && strcmp(key, table_entry->key) == 0) {
				free(valid_symbol_types);
				return table_entry;
			}
		}
	} while ((table_entry = table_entry->next) != NULL);
	/* not found, return NULL */
	free(valid_symbol_types);
	return NULL;
}