#ifndef PARSER_H_
#define PARSER_H_

#include "list.h"
#include <stdio.h>

#define NewLineData(_line_data) _line_data = (LineData*)malloc(sizeof(LineData));\
	if (!_line_data) error_fatal(ErrorMemoryAlloc);\
	_line_data->decimal_address = 0;\
	_line_data->machine_code.code = 0;\
	_line_data->label_to_extract = NULL;\
	_line_data->is_instruction = 0;\
	_line_data->are = 0;

#define NewLabel(_label) _label = (Label*)malloc(sizeof(Label));\
	if (!_label) error_fatal(ErrorMemoryAlloc);\
	_label->label = NULL;\
	_label->line = 0;\
	_label->label_type = 0;

#define MaxLabelSize 30
#define MaxLineSize 80
#define CommandsAmount 16

/* 2^19 - 1 = 524287 */
#define MaxDataNumber 524287
/* 2^19 - 2^20 = -524288 */
#define MinDataNumber -524288
/* 2^20 = 1048576, for converting  negatives into two's complement. */
#define Complement 1048576
/* 2*MAX SIZE OF LABEL = 2*30 + 2 FOR {} + 1 FOR END OF TEXT '\0' */
#define MaxRegisterNumber 7

typedef enum {LineTypeEntry, LineTypeExtern} LineType;

typedef enum {LabelTypeCommand, LabelTypeData, LabelTypeExtern} LabelType;

/**
 * Assembler command set.
 * Addressing code are 0 not in use, 1 in use.
 * src_operand and dest_operand are 0 if not in use , and 1 if in use.
 */
typedef struct {
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

/**
 * Holds the 20 bit machine code.
 */
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
} MachineCodeBits;

/**
 * Union for holding the machine code with both integer and bits
 * representation.
 */
typedef union { 
	MachineCodeBits bits;
	unsigned long code;
} MachineCode;

/**
 * Structure for holding all the data gathered on a line.
 */
typedef struct {
	int decimal_address;
	/* a for absolute, r for relocatable, e for external. */
	char are;
	char* label_to_extract;
	MachineCode machine_code;
	unsigned int is_instruction :1;
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
	unsigned int errors;
	/* Command and data counters. */
	unsigned int IC;
	unsigned int DC;
} ParserData;

extern ParserData parser_data;

/**
 * Does the initial parsing of the assembly file.
 * A file must be opened using reader
 */
int parse();

/**
 * Free all of the parser variables.
 */
void parser_clean();

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

#endif /* PARSER_H_ */
