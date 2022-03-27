#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include "code.h"
#include "utils.h"


/**
 * Validates the operands addressing types, and prints error message if needed.
 * @param line The current line information
 * @param op1_addressing The current addressing of the first operand
 * @param op2_addressing The current addressing of the second operand
 * @param op1_valid_addr_count The count of valid addressing types for the first operand
 * @param op2_valid_addr_count The count of valid addressing types for the first operand
 * @param ... The valid addressing types for first & second operand, respectively
 * @return Whether addressign types are valid
 */
static bool validate_operand_addresing(line_descriptor line, addressing_type op1_addressing, addressing_type op2_addressing,
                                       int op1_valid_addr_count, int op2_valid_addr_count, ...);


bool analyze_operands(line_descriptor line, int i, char **destination, int *operand_count) {
	int j;
	*operand_count = 0;
	destination[0] = destination[1] = NULL;
	SKIP_TO_NEXT_NON_WHITESPACE(line.content, i)
	if (line.content[i] == ',') {
        fprintf_error_specific(line, "[ERROR] Unexpected comma after command.");
		return FALSE; /* an error occurred */
	}

	/* Until not too many operands (max of 2) and it's not the end of the line */
	for (*operand_count = 0; line.content[i] != EOF && line.content[i] != '\n' && line.content[i];) {
		if (*operand_count == 2) /* =We already got 2 operands in, We're going ot get the third! */ {
            fprintf_error_specific(line, "[ERROR] Too many operands for operation (got >%d)", *operand_count);
			free(destination[0]);
			free(destination[1]);
			return FALSE; /* an error occurred */
		}

		/* Allocate memory to save the operand */
		destination[*operand_count] = better_malloc(MAX_LINE_LENGTH);
		/* as long as we're still on same operand */
		for (j = 0; line.content[i] && line.content[i] != '\t' && line.content[i] != ' ' && line.content[i] != '\n' && line.content[i] != EOF &&
		            line.content[i] != ','; i++, j++) {
			destination[*operand_count][j] = line.content[i];
		}
		destination[*operand_count][j] = '\0';
		(*operand_count)++; /* We've just saved another operand! */
		SKIP_TO_NEXT_NON_WHITESPACE(line.content, i)

		if (line.content[i] == '\n' || line.content[i] == EOF || !line.content[i]) break;
		else if (line.content[i] != ',') {
			/* After operand & after white chars there's something that isn't ',' or end of line.. */
            fprintf_error_specific(line, "[ERROR] Only whitespace and comma supposed to separate operands");
			/* Release operands dynamically allocated memory */
			free(destination[0]);
			if (*operand_count > 1) {
				free(destination[1]);
			}
			return FALSE;
		}
		i++;
		SKIP_TO_NEXT_NON_WHITESPACE(line.content, i)
		/* if there was just a comma, then (optionally) white char(s) and then end of line */
		if (line.content[i] == '\n' || line.content[i] == EOF || !line.content[i])
            fprintf_error_specific(line, "[ERROR] Missing operand after comma.");
		else if (line.content[i] == ',') fprintf_error_specific(line, "[ERROR] Consecutive commas.");
		else continue; /* No errors, continue */
		{ /* Error found! (didn't continue) */
			/* No one forgot you two! */
			free(destination[0]);
			if (*operand_count > 1) {
				free(destination[1]);
			}
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * ABSOLUTE single lookup table element
 */
struct cmd_lookup_element {
	char *cmd;
	opcode op;
	funct fun;
};
/**
 * ABSOLUTE lookup table for opcode & funct by command name
 */
static struct cmd_lookup_element lookup_table[] = {
		{"mov", MOV_OP, FUNCT_DEFAULT},
		{"cmp",CMP_OP, FUNCT_DEFAULT},
		{"add",ADD_OP, FUNCT_ADD},
		{"sub",SUB_OP, FUNCT_SUB},
		{"lea",LEA_OP, FUNCT_DEFAULT},
		{"clr",CLR_OP, FUNCT_CLR},
		{"not",NOT_OP, FUNCT_NOT},
		{"inc",INC_OP, FUNCT_INC},
		{"dec",DEC_OP, FUNCT_DEC},
		{"jmp",JMP_OP, FUNCT_JMP},
		{"bne",BNE_OP, FUNCT_BNE},
		{"jsr",JSR_OP, FUNCT_JSR},
		{"red",RED_OP, FUNCT_DEFAULT},
		{"prn",PRN_OP, FUNCT_DEFAULT},
		{"rts",RTS_OP, FUNCT_DEFAULT},
		{"stop",STOP_OP, FUNCT_DEFAULT},
		{NULL, DEFAULT_OP, FUNCT_DEFAULT}
};
void get_opcode_func(char *cmd, opcode *opcode_out, funct *funct_out) {
	struct cmd_lookup_element *e;
	*opcode_out = DEFAULT_OP;
	*funct_out = FUNCT_DEFAULT;
	/* iterate through the lookup table, if commands are same return the opcode of found. */
	for (e = lookup_table; e->cmd != NULL; e++) {
		if (strcmp(e->cmd, cmd) == 0) {
			*opcode_out = e->op;
			*funct_out = e->fun;
			return;
		}
	}
}

addressing_type get_addressing_type(char *operand) {
    int open_braces_index;
    int closing_braces_index;
    /* if nothing, just return none */
	if (operand[0] == '\0'){
        return NONE_ADDR;
    }

	/* if first char is 'r', second is number in range 0-19 and third is end of string, it's a register */

	if (get_regular_register_by_name(operand) != NONE_REGISTER){
        return REGISTER_ADDR;
    }
    /*if operand starts with # and follows with base10 number => Immediate addressing */
    if (operand[0] == '#' && is_integer(operand + 1)) {
        return IMMEDIATE_ADDR;
    }
    /* if operand is a valid label name, it's directly addressed */
    if (is_valid_label_name(operand)) {
        return DIRECT_ADDR;
    }

    if(get_index_register_by_name(operand) != NONE_REGISTER){
        return INDEX_ADDR;
    }

    return NONE_ADDR;
}

/**
 * Validates the operands' addressing types by the opcode of the instruction
 * @param line Current processed line
 * @param first_addressing First operand addressing
 * @param second_addressing Second operand addressing
 * @param curr_opcode Opcode
 * @param operands_count Operands count
 * @return True if addressing is ok else fasle.
 */
bool validate_opcode_operands(line_descriptor line, addressing_type first_addressing,
                              addressing_type second_addressing, opcode curr_opcode, int operands_count) {
	if ( MOV_OP <= curr_opcode && curr_opcode <= LEA_OP) {
		/* 2 operands required */
		if (operands_count != 2) {
            fprintf_error_specific(line, "[ERROR] Opcode specifies usage 2 operands not %d", operands_count);
			return FALSE;
		}
		/* validate operand addressing */
		if (curr_opcode == CMP_OP) {
			return validate_operand_addresing(line, first_addressing, second_addressing, 4, 4,
                                              IMMEDIATE_ADDR, DIRECT_ADDR, INDEX_ADDR, REGISTER_ADDR,
                                              IMMEDIATE_ADDR, DIRECT_ADDR, INDEX_ADDR, REGISTER_ADDR);
		} else if (curr_opcode == MOV_OP || curr_opcode == SUB_OP) { /* ADD == SUB */
			return validate_operand_addresing(line, first_addressing, second_addressing, 4, 3,
                                              IMMEDIATE_ADDR, DIRECT_ADDR, INDEX_ADDR, REGISTER_ADDR,
                                              DIRECT_ADDR, INDEX_ADDR, REGISTER_ADDR);
		} else if (curr_opcode == LEA_OP) {

			return validate_operand_addresing(line, first_addressing, second_addressing, 2, 3,
                                              DIRECT_ADDR, INDEX_ADDR,
                                              DIRECT_ADDR, INDEX_ADDR, REGISTER_ADDR);
		}
	} else if (CLR_OP <= curr_opcode  && curr_opcode <= PRN_OP) {
		/* Following opcodes specify usage of single parameter */
		if (operands_count != 1) {
			fprintf_error_specific(line, "[ERROR] Opcode specifies usage single operand not %d", operands_count);
			return FALSE;
		}
		/* validate operand addressing */
		if (curr_opcode ==  INC_OP || curr_opcode == RED_OP) { /* NOT == INC == CLR == DEC */
			return validate_operand_addresing(line, first_addressing, NONE_ADDR, 3, 0, DIRECT_ADDR, INDEX_ADDR,
                                              REGISTER_ADDR);
		} else if (curr_opcode == BNE_OP) { /* BNE == JSR == JMP */
			return validate_operand_addresing(line, first_addressing, NONE_ADDR, 2, 0, DIRECT_ADDR, INDEX_ADDR);
		} else /* if (curr_opcode == PRN_OP) */ {
			return validate_operand_addresing(line, first_addressing, NONE_ADDR, 4, 0, IMMEDIATE_ADDR, DIRECT_ADDR,
                                              INDEX_ADDR, REGISTER_ADDR);
		}
	} else if (curr_opcode <= STOP_OP && curr_opcode >= RTS_OP) {
		/* 0 operands exactly */
		if (operands_count != 0) {
            fprintf_error_specific(line, "[ERROR] Opcode specifies usage 0 operands not %d", operands_count);
			return FALSE;
		}
	}
	return TRUE;
}


int encode_opdcode_wards(line_descriptor line, opcode line_opcode, funct line_funct, int op_count, char *operands[2],
                         opcode_word** opcode_encode, operand_word** operand_encode) {
	/* Get addressing types and validate them: */
	addressing_type first_addressing = op_count >= 1 ? get_addressing_type(operands[0]) : NONE_ADDR;
	addressing_type second_addressing = op_count == 2 ? get_addressing_type(operands[1]) : NONE_ADDR;
	/* validate operands by opcode - on failure exit */
	if (!validate_opcode_operands(line, first_addressing, second_addressing, line_opcode, op_count)) {
		return 0;
	}
	/* Create the code word by the data: */

    (*opcode_encode) = (opcode_word*) better_malloc(sizeof(opcode_word));
    (*opcode_encode)->opcode = line_opcode;
    (*opcode_encode)->ARE= ABSOLUTE;
    (*opcode_encode)->placeholder = 0;

    if(line_opcode != RTS_OP && line_opcode != STOP_OP){
        (*operand_encode) = (operand_word*) better_malloc(sizeof (operand_word));
        (*operand_encode)->funct = line_funct;
        (*operand_encode)->ARE= ABSOLUTE;
        (*operand_encode)->placeholder = 0;

        /* Zeroing for good measure */
        (*operand_encode)->destination_addressing = 0;
        (*operand_encode)->destination_register = 0;
        (*operand_encode)->source_addressing = 0;
        (*operand_encode)->source_register = 0;
    }
    else{
        (*operand_encode) = NULL;
    }

    if(MOV_OP <= line_opcode && line_opcode <= LEA_OP){
        (*operand_encode)->destination_addressing = second_addressing;
        (*operand_encode)->source_addressing = first_addressing;

        if(second_addressing == REGISTER_ADDR || second_addressing == INDEX_ADDR ){
            (*operand_encode)->destination_register = get_register_by_name_and_addressing(operands[1],second_addressing);
        }
        if(first_addressing == REGISTER_ADDR || first_addressing == INDEX_ADDR ){
            (*operand_encode)->source_register = get_register_by_name_and_addressing(operands[0],first_addressing);
        }
    }

    else if (CLR_OP <= line_opcode && line_opcode <=PRN_OP){
        (*operand_encode)->destination_addressing = first_addressing;
        if(first_addressing == REGISTER_ADDR || first_addressing == INDEX_ADDR ){
            (*operand_encode)->destination_register = get_register_by_name_and_addressing(operands[0],first_addressing);
        }
    }

	return 2;
}

static bool validate_operand_addresing(line_descriptor line, addressing_type op1_addressing, addressing_type op2_addressing, int op1_valid_addr_count,
                                       int op2_valid_addr_count, ...) {
	int i;
	bool is_valid;
	va_list list;

	addressing_type op1_valids[4], op2_valids[4];
	memset(op1_valids, NONE_ADDR, sizeof(op1_valids));
	memset(op2_valids, NONE_ADDR, sizeof(op2_valids));

    /* init arg list */
	va_start(list, op2_valid_addr_count);
	/* populate the argument list for both the operands  */
	for (i = 0; i < op1_valid_addr_count && i <= 3 ;i++)
    {
		op1_valids[i] = va_arg(list, int);
    }

	/* Again for second operand by the count */
	for (i = 0; i < op2_valid_addr_count && i <= 3 ;i++)
		op2_valids[i] = va_arg(list, int);

	va_end(list);

	/* Check if addressing we got is an allowed one */
	is_valid = op1_valid_addr_count == 0 && op1_addressing == NONE_ADDR;
	for (i = 0; i < op1_valid_addr_count && !is_valid; i++) {
		if (op1_valids[i] == op1_addressing) {
			is_valid = TRUE;
		}
	}
	if (!is_valid) {
        fprintf_error_specific(line, "[ERROR] Wrong addressing for the 1st operand");
		return FALSE;
	}
	/* Same */
	is_valid = op2_valid_addr_count == 0 && op2_addressing == NONE_ADDR;
	for (i = 0; i < op2_valid_addr_count && !is_valid; i++) {
		if (op2_valids[i] == op2_addressing) {
			is_valid = TRUE;
		}
	}
	if (!is_valid) {
        fprintf_error_specific(line, "[ERROR] Wrong addressing for the 2nd operand");
		return FALSE;
	}
	return TRUE;
}


reg get_regular_register_by_name(char *name) {
    int name_len = strlen(name);

    if (2<= name_len && name_len <=3 && name[0] == REGISTER_PREFIX) {
        char *reg_num_str = &name[1];

        /* validate we are working only with digits due to atoi limitations */
        if (isdigit(reg_num_str[0]) &&( name_len == 2 || isdigit(reg_num_str[1]))) {
            int reg_num_int = atoi(reg_num_str);
            /* registers are numbered between 0 to MAX_REGISTER  */
            if (0 <= reg_num_int && reg_num_int <= MAX_REGISTER) {
                return reg_num_int;
            }
        }
    }
	return NONE_REGISTER; /* No match */
}

reg get_index_register_by_name(char *operand) {

    int open_braces_index = index_of_char(operand, '[');
    int closing_braces_index = index_of_char(operand, ']');

    /* test for ....[XX]<-- */
    if (open_braces_index != -1 && closing_braces_index !=-1){
        char *register_str = operand+open_braces_index+1;
        reg reg_num;
        operand[open_braces_index] = '\0'; /* separating to test only the label*/
        operand[closing_braces_index] = '\0'; /* closing the second string */
        reg_num = get_regular_register_by_name(register_str);


        if(is_valid_label_name(operand) && 10<=reg_num && reg_num <= 15 ){
            return reg_num;
        }
    }
    return NONE_REGISTER; /* No match */
}

reg get_register_by_name_and_addressing(char *name, addressing_type addr_type){
    if(addr_type == INDEX_ADDR){
        return get_index_register_by_name(name);
    }
    return get_regular_register_by_name(name);
}


data_word *encode_operand_data_old(addressing_type addressing, long data, bool is_extern_symbol) {
	signed long mask; /* For bitwise operations for data conversion */
	unsigned long ARE = 4, mask_un; /* 4 = 2^2 = 1 << 2 */
	data_word *dataword = better_malloc(sizeof(data_word));

	if (addressing == DIRECT_ADDR) {
		ARE = is_extern_symbol ? 1 : 2;
	}
	dataword->ARE = ARE; /* Set ARE field value */

	/* Now all left is to encode the data */
	mask = -1;
	mask_un = mask; /* both hold 11...11 */
	mask_un >>= 11; /* Now mask_un holds 0..001111....111, 11 zeros and 21 ones */
	dataword->data = mask_un & data; /* Now just the 21 lsb bits area left and assigned to data field. */
	return dataword;
}

operand_data_word * encode_operand_data(addressing_type addressing, int data, bool external_symbol) {
    operand_data_word *operand_data_temp = (operand_data_word*)better_malloc(sizeof(operand_data_word));

    if(addressing == DIRECT_ADDR || addressing == INDEX_ADDR){
        if(external_symbol){
            operand_data_temp->ARE = EXTERNAL;
        } else{
            operand_data_temp->ARE = RELOCATABLE;
        }
    } else{
        operand_data_temp->ARE = ABSOLUTE;
    }
    operand_data_temp->data = data;

    return operand_data_temp;
}