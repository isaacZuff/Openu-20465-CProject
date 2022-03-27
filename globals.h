/* Constants */

#ifndef _GLOBALS_H
#define _GLOBALS_H

/** Boolean (t/f) definition */
typedef enum enum_bool{
	FALSE,
    TRUE
} bool;

/** Maximum size of code image and data2 image */
#define CODE_ARR_IMG_LENGTH 1200

/** Maximum length of a single source line  */
#define MAX_LINE_LENGTH 80

/** Maximum length of a single source line  */
# define MAX_LABEL_LENGTH 31

/** Initial IC value */
#define IC_INIT_VALUE 100

#define REGISTER_PREFIX 'r'

#define MAX_REGISTER 15

#define MACHINE_WORD_LENGTH 20

/* Note: many enum declaration contains NONE_X value - which is a flag for not found during parsing. */

typedef enum object_file_mask{
    E_MASK = 15,
    D_MASK = 240,
    C_MASK = 3840,
    B_MASK = 61440,
    A_MASK = 983040

} ob_file_mask;


/** Operand addressing type */
typedef enum addressing_types {
	/** Immediate addressing (0) */
	IMMEDIATE_ADDR = 0,
	/** Direct addressing (1) */
	DIRECT_ADDR = 1,
	/** Index addressing (2) */
	INDEX_ADDR = 2,
	/** Register addressing */
	REGISTER_ADDR = 3,
	/** "DEFAULT" */
	NONE_ADDR = -1
} addressing_type;

/** Commands opcode */
typedef enum opcodes {
	/* First opcode group */
	MOV_OP = 0,
	CMP_OP = 1,

	ADD_OP = 2,
	SUB_OP = 2,

	LEA_OP = 4,

	/* Second opcode group */
	CLR_OP = 5,
	NOT_OP = 5,
	INC_OP = 5,
	DEC_OP = 5,

	JMP_OP = 9,
	BNE_OP = 9,
	JSR_OP = 9,

	RED_OP = 12,
	PRN_OP = 13,

	/* Third opcode group */
	RTS_OP = 14,
	STOP_OP = 15,

	/* "Default" */
	DEFAULT_OP = -1
} opcode;

/** Commands funct */
typedef enum funct {
	/* OPCODE 2 */
	FUNCT_ADD = 10,
	FUNCT_SUB = 11,

	/* OPCODE 5 */
	FUNCT_CLR = 10,
    FUNCT_NOT = 11,
    FUNCT_INC = 12,
    FUNCT_DEC = 13,

	/* OPCODE 9 */
    FUNCT_JMP = 10,
    FUNCT_BNE = 11,
    FUNCT_JSR = 12,

	/* DEFAULT */
    FUNCT_DEFAULT = 0
} funct;

/** Registers - r0->r1 + not found */
typedef enum registers {
	R0 = 0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
	NONE_REGISTER = -1
} reg;

typedef enum are_options {
    EXTERNAL=0,
    RELOCATABLE,
    ABSOLUTE
} are;
/** Represents a single code word */
typedef struct REPLACE {
    unsigned int destination_addressing: 2;
    unsigned int destination_register: 4;
    unsigned int source_addressing: 2;
    unsigned int source_register: 4;
    unsigned int funct: 4;
    unsigned int ARE: 3;
    unsigned int placeholder: 1;

} REPLACE;

typedef struct operand_code_ward_options {
    unsigned int destination_addressing: 2;
    unsigned int destination_register: 4;
    unsigned int source_addressing: 2;
    unsigned int source_register: 4;
    unsigned int funct: 4;
    unsigned int ARE: 3;
    unsigned int placeholder: 1;

} operand_word;

typedef struct opcode_word {
    unsigned int opcode: 16;
    unsigned int ARE: 3;
    unsigned int placeholder: 1;

} opcode_word;

/** Represents a single data word. */
typedef struct data_word {
	unsigned int ARE: 3;
	/* The data content itself (a method for putting data into these field is defined) */
	unsigned long data;
} data_word;

typedef struct operand_data_word {
    unsigned int ARE: 3;
    /* The data content itself (a method for putting data into these field is defined) */
    unsigned int data:16;
    unsigned int placeholder: 1;
} operand_data_word;

/** Represents a general machine code word contents */
typedef struct machine_word {
	/* if it represents code (not additional data), this field contains the total length required by the code. if it's data, this field is 0. */
	long length;
    bool is_operand;
	/* The content can be code or data */
	union word {
		data_word *data;
        operand_data_word *data2;
        opcode_word* opcode;
        operand_word* operand;
		REPLACE *code;
	} word;
} machine_word;

/** Instruction type (.data, .entry, etc.) */
typedef enum instruction {
	/** .data instruction */
	DATA_INST,
	/** .extern instruction */
	EXTERN_INST,
	/** .entry instruction */
	ENTRY_INST,
	/** .string instruction */
	STRING_INST,
	/** Not found */
	NONE_INST,
	/** Parsing/syntax error */
	ERROR_INST
} instruction;

/**
 * Represents a single source line, including it's details
 */
typedef struct line_info {
	/** Line number in file */
	long line_number;
	/** File name */
	char *file_name;
	/** Line content (source) */
	char *content;
} line_descriptor;

#endif
