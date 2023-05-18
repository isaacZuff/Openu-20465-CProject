#ifndef ASSEMBLER_LINKEDLIST_H
#define ASSEMBLER_LINKEDLIST_H
#include "globals.h"


/***
 * Inserts node at the start of the list
 * @param head_ref pointer to list head
 * @param new_data string
 */
void insert_at_the_head(struct list_node** head_ref, char* new_data);

/***
 * Inserts node at the end of the list
 * @param head_ref  pointer to list head
 * @param new_data string
 */
list_node * insert_node_list_at_the_end(struct list_node** head_ref, char* new_data);

/***
 * Insert new node at the end of linked list string or creates the list
 * @param head_ref head if exists null if not
 * @param new_data string to add
 */
void insert_string_node_at_the_end(simple_node ** head_ref, char* new_data);

/**
 * creates linked list node
 * @param new_data string
 * @return new object created
 */
list_node *create_list_node(char* new_data);

/***
 * finds field in list
 * @param field_to_find
 * @return If found --> node, Else --> NULL
 */
list_node *find_node_in_list(list_node* node, char* field_to_find);


/***
 * Free memory allocated for node
 * @param node node to free
 */
void free_string_node(simple_node** node);

#endif //ASSEMBLER_LINKEDLIST_H
