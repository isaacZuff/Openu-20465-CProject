#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pre_assembler.h"
#include "globals.h"
#include "helper.h"
#include "linkedlist.h"
#include "output_module.h"

void expand_macros(char* filename){
    char *filename_with_ext;
    FILE *file_des;
    char current_line[MAX_LINE_LENGTH + 2];
    char field[MAX_LINE_LENGTH+2];
    bool is_macro = FALSE;
    list_node* macro_names_list =NULL;
    list_node *current_macro_to_add = NULL;
    simple_node* new_file_lines =NULL;
    //simple_node* macro_lines_temp = NULL;


    filename_with_ext = strcat_to_new(filename, PRE_MARCO_SUFFIX);

    /* Try to open file, if something wrong skip */
    if ((file_des = fopen(filename_with_ext, "r")) == NULL) {
        /* if file couldn't be opened, write to stderr. */
        printf_error("[ERROR] Unable to read file: %s\n", filename);
        free(filename_with_ext); /*free the memory we allocated to the string concat */
        return;
    }

    /* We'll iterate line by line and write non macro lines to the filename.POST_MACRO_SUFFIX file */
    /* Remember there are no check for line integrity in this step*/

    while (fgets(current_line, MAX_LINE_LENGTH + 2, file_des) != NULL) {
        int index = 0;
        list_node *current_node = NULL;

        SKIP_TO_NEXT_NON_WHITESPACE(current_line, index);
        /* Detect if we are in a comment or an empty line */
        if (!current_line[index] || current_line[index] == '\n' || current_line[index] == EOF ||
            current_line[index] == ';') {
            insert_string_node_at_the_end(&new_file_lines,current_line);
            continue;
        }

        index = get_first_field(current_line, field);

        if(strcmp("endm",field) == 0 ){
            is_macro =FALSE;
        } else if(is_macro){
            insert_string_node_at_the_end(&(current_macro_to_add->macro_lines),current_line);
        } else if(strcmp("macro",field) == 0) {
            is_macro = TRUE;
            get_first_field(current_line+index,field);
            current_macro_to_add = insert_node_list_at_the_end(&macro_names_list, field);
        }

        else if ((current_node = find_node_in_list(macro_names_list,field)) !=NULL){
            simple_node *perv;
            simple_node *macro_lines_temp = current_node->macro_lines;
            while(macro_lines_temp != NULL){
                insert_string_node_at_the_end(&new_file_lines,macro_lines_temp->data);
                perv = macro_lines_temp;
                macro_lines_temp = macro_lines_temp->next;
                free_string_node(&perv);
            }
        } else{
            insert_string_node_at_the_end(&new_file_lines,current_line);
        }

    }

    /* Write the macro to file with POST_MACRO_SUFFIX */
    write_macro_file(new_file_lines,filename);
}