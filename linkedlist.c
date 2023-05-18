// Linked list operations in C

#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "string.h"
#include "globals.h"
#include "helper.h"

/***
 * Inserts node at the start of the list
 * @param head_ref pointer to list head
 * @param new_data string
 */
void insert_at_the_head(struct list_node** head_ref, char* new_data) {
    // Allocate memory to a node
    list_node *new_node = (list_node*)better_malloc(sizeof(list_node));

    // insert the data
    new_node->data = strdup(new_data);
    new_node->next = (*head_ref);
    new_node->macro_lines = NULL;

    // Move head to new node
    (*head_ref) = new_node;
}


/***
 * Inserts node list "object" at the end of the list
 * @param head_ref  pointer to list head
 * @param new_data string
 * @returns the_new_node_created
 */
list_node * insert_node_list_at_the_end(list_node** head_ref, char* new_data) {
    list_node *new_node = (list_node*)better_malloc(sizeof(list_node));
    list_node *last; /* used in step 5*/

    new_node->data = strdup(new_data);
    new_node->next = NULL;
    new_node->macro_lines = NULL;

    if (*head_ref == NULL) {
        *head_ref = new_node;
        return new_node;
    }
    last = *head_ref;

    while (last->next != NULL) last = last->next;

    last->next = new_node;
    return new_node;
}

/***
 * Insert new node at the end of linked list string or creates the list
 * @param head_ref head if exists null if not
 * @param new_data string to add
 */
void insert_string_node_at_the_end(simple_node ** head_ref, char* new_data) {
    simple_node *new_node = (simple_node*)better_malloc(sizeof(simple_node));
    simple_node *last; /* used in step 5*/

    new_node->data = strdup(new_data);
    new_node->next = NULL;

    if (*head_ref == NULL) {
        *head_ref = new_node;
        return;
    }
    last = *head_ref;

    while (last->next != NULL) last = last->next;

    last->next = new_node;
}

/**
 * creates linked list node
 * @param new_data string
 * @return new object created
 */
list_node *create_list_node(char* new_data){
    list_node *new_node = (list_node*)malloc(sizeof(list_node));
    new_node->data = strdup(new_data);
    new_node->next = NULL;
    return new_node;
}

/***
 * finds field in list
 * @param field_to_find
 * @return If found --> node, Else --> NULL
 */
list_node *find_node_in_list(list_node* node, char* field_to_find){
    list_node* current_node = node;

    while (current_node != NULL){
        if(strcmp(current_node->data,field_to_find) == 0){
            return current_node;
        }
        current_node = current_node->next;
    }
    return NULL;
}

/***
 * Free memory allocated for node
 * @param node node to free
 */
void free_string_node(simple_node** node){
    if(*node == NULL){
        if((*node)->data != NULL) {
            free((*node)->data);
        }
        free((*node));
    }
}
