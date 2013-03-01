/*
 * parser.h
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>

#define New(type) (type *)malloc(sizeof(type))

#define MAX_LABEL_SIZE 30
/* =2^19-1 */
#define MAX_DATA_NUMBER 524287 
/* =2^20-2^19 */
#define MIN_DATA_NUMBER -524288 
#define LINE_OFFSET 100
/* 2*MAX SIZE OF LABEL = 2*30 + 2 FOR {} + 1 FOR END OF TEXT '\0' */

#define OpenFile(file, extension) {\
	char* file_name = reader_get_file_name(extension);\
	file = fopen(file_name, "w");\
	if (!file) {\
		fprintf(stderr, ErrorCantRead, file_name);\
		fprintf(stderr, "\n");\
		free(file_name);\
		exit(EXIT_FAILURE);\
	}\
	free(file_name);\
}

typedef enum {LINE_TYPE_ENTRY, LINE_TYPE_EXTERN} LineType;

typedef enum {LABEL_TYPE_COMMAND, LABEL_TYPE_DATA, LABEL_TYPE_EXTERN} LabelType;

/* Output files. */
enum {ENT_FILE, EXT_FILE, OB_FILE};

/**
 * Assembler commands set.
 * Addressing code are 0 not in use, 1 in use
 * source operand and destination_operand are 0 if not in use , and 1 if in use
 */
typedef struct {
	unsigned int source_imidiat_addressing :1;
	unsigned int source_direct_addressing :1;
	unsigned int source_index_addressing :1;
	unsigned int source_direct_register_addressing :1;
	unsigned int destination_imidiat_addressing :1;
	unsigned int destination_direct_addressing :1;
	unsigned int destination_index_addressing :1;
	unsigned int destination_direct_register_addressing :1;
	unsigned int source_operand :1;
	unsigned int destination_operand :1;
	char *command;
} Command;

typedef struct {
	unsigned int comb :2; /*active only if type=1*/
	unsigned int destination_register :3;
	unsigned int destination_addressing :2;
	unsigned int source_register :3;
	unsigned int source_addressing :2;
	unsigned int opcode :4;
	unsigned int type :1;
	unsigned int unused :3;
} command_word;

typedef union { 
	command_word inst;
	unsigned long data;
} word;

typedef struct {
	int decimal_address;
	char *label_to_extract;
	word line_word;
} LineData;

/**
 * The data stored on a labels-list's node.
 */
typedef struct {
	char* label;
	unsigned int line; 
	LabelType label_type;
} Label;

/**
 * Does the initial parsing of the assembly file.
 * A file must be opened using reader
 */
void parser_parse();

/**
 * Performs "Second phase" commands translation.
 */
void parser_translate_commands();

/**
 * Create the entry symbols file.
 */
void parser_create_ent_file();

/**
 * Check whether a line starts with a label.
 * Returns the label as a string. The string must be freed by the invoker.
 */
char* parser_get_label(const char* line, int line_num);

/**
 * Callback function for list_add_ordered(); Performs lexicographical comparison 
 * of two labels
 * 
 * @param a
 *   Pointer to Label.
 * @param Label b
 *   Pointer to Label.
 * 
 * @return 
 *   positive number if the first label's name is "larger" than the second's 
 *   name.  
 */
int _parser_compare_labels(void* a, void* b);

/**
 * Callback function for list_add_ordered(); Issues error message when a 
 * duplicated label is declared.
 *  
 * @param data
 *   Pointer to Label.
 */
void _parser_duplicated_label(void* data);

/**
 * Callback function for list_print(). Prints one label list item.
 * 
 * @param data
 *   Pointer to label.
 * @param stream
 *   File to write to.
 */
void _parser_print_label(void* data, FILE* stream);

/**
 *
 */
void _parser_print_data_item(void* data, FILE* stream);

/**
 *
 */
void _parser_find_data_item_label(void* data);

/**
 *
 */
void _parser_find_label_address(void* data);

/**
 *
 */
void _parser_translate_command(void* data);

#endif /* PARSER_H_ */
