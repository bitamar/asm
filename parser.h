/*
 * parser.h
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "list.h"
#include <stdio.h>

#define NewLineData(_line_data) _line_data = (LineData*)malloc(sizeof(LineData));\
	if (!_line_data) error_fatal(ErrorMemoryAlloc);\
	_line_data->decimal_address = 0;\
	_line_data->line_word.data = 0;\
	_line_data->label_to_extract = NULL;\
	_line_data->line_type = 0;

#define NewLabel(_label) _label = (Label*)malloc(sizeof(Label));\
	if (!_label) error_fatal(ErrorMemoryAlloc);\
	_label->label = NULL;\
	_label->line = 0;\
	_label->label_type = 0;

#define MAX_LABEL_SIZE 30
/* 2^19 - 1 = 524287 */
#define MAX_DATA_NUMBER 524287 
/* 2^19 - 2^20 = -524288 */
#define MIN_DATA_NUMBER -524288 
#define LINE_OFFSET 100
/* 2^20 = 1048576, for converting  negatives into two's complement. */
#define MINUS 1048576
/* 2*MAX SIZE OF LABEL = 2*30 + 2 FOR {} + 1 FOR END OF TEXT '\0' */

typedef enum {LINE_TYPE_ENTRY, LINE_TYPE_EXTERN} LineType;

typedef enum {LABEL_TYPE_COMMAND, LABEL_TYPE_DATA, LABEL_TYPE_EXTERN} LabelType;

/* Output files. */
enum {ENT_FILE, EXT_FILE, OB_FILE};

/**
 * Assembler command set.
 * Addressing code are 0 not in use, 1 in use.
 * src_operand and dest_operand are 0 if not in use , and 1 if in use.
 */
typedef struct {
	char *command;
	unsigned int src_imidiate_address :1;
	unsigned int src_direct_address :1;
	unsigned int src_index_address :1;
	unsigned int src_direct_reg_address :1;
	unsigned int dest_imidiate_address :1;
	unsigned int dest_direct_address :1;
	unsigned int dest_index_address :1;
	unsigned int dest_direct_reg_address :1;
	unsigned int src_operand :1;
	unsigned int dest_operand :1;
} Command;

typedef struct {
	/* Comb is relevant only when type is 1. */
	unsigned int comb :2;
	unsigned int dest_reg :3;
	unsigned int dest_address :2;
	unsigned int src_reg :3;
	unsigned int src_address :2;
	unsigned int opcode :4;
	unsigned int type :1;
	unsigned int unused :3;
} CommandWord;

typedef union { 
	CommandWord inst;
	unsigned long data;
} Word;

typedef struct {
	int decimal_address;
	/* a for absolute, r for relocatable, e for external. */
	char are;
	char *label_to_extract;
	Word line_word;
	unsigned int line_type :1;
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
 * Wrapper for parser data.
 */
typedef struct {
	List parser_symbols;
	List parser_entry_symbols;
	/* List for data. */
	List data_list;
	/* List for commands. */
	List commands_list;
	/* Parsing errors counter, for avoiding the second iteration when errors are
	 * found. */
	char parser_errors;
	/* Command and data counters. */
	int IC;
	int DC;
} ParserData;

extern ParserData parser_data;

void extract_data_number(char*, int);
int extract_string(char*, int, char*);
void extract_label(char* begin_of_word, char *end_of_word, const int line_num, char * line, LineType line_type);
long extract_number(char*, const int);
int extract_operand(char*, int i, int line_num);
int extract_operand_offset(char* ,int i, int line_num);
int update_operand(LineData* line_data, char *operand,char *operand_offset,int work_on_src);
int add_operand_lines (char*, char*, int, int, int, int);

/**
 * Does the initial parsing of the assembly file.
 * A file must be opened using reader
 */
ParserData* parser_parse();


/**
 *
 */
void parser_clean();

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

void print_data_list(List list);

#endif /* PARSER_H_ */
