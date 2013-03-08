#include "error.h"
#include "list.h"
#include "parser.h"
#include "reader.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Parser data, to be passed to the second phase translator. */
ParserData parser_data;

/* Commands definition. */
const Command commands[] = {
	{"mov", 1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
	{"cmp", 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{"add", 1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
	{"sub", 1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
	{"not", 0, 0, 0, 0, 0, 1, 1, 1, 0, 1},
	{"clr", 0, 0, 0, 0, 0, 1, 1, 1, 0, 1},
	{"lea", 0, 1, 1, 1, 0, 1, 1, 1, 1, 1},
	{"inc", 0, 0, 0, 0, 0, 1, 1, 1, 0, 1},
	{"dec", 0, 0, 0, 0, 0, 1, 1, 1, 0, 1},
	{"jmp", 0, 0, 0, 0, 0, 1, 1, 1, 0, 1},
	{"bne", 0, 0, 0, 0, 0, 1, 1, 1, 0, 1},
	{"red", 0, 0, 0, 0, 0, 1, 1, 1, 0, 1},
	{"prn", 0, 0, 0, 0, 1, 1, 1, 1, 0, 1},
	{"jsr", 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
	{"rts", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{"stop", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

/**
 * Current char holds the current char in the line being read.
 */
char* current_char;

/**
 * Call-back function for list_destruct. Free a label.
 */
void _parser_free_label(void* data) {
	Label* label = data;
	free(label->label);
}

/**
 * Call-back function for list_destruct. Free one line data.
 */
void _parser_free_line_data(void* data) {
	LineData* line_data = data;
	free(line_data->label_to_extract);
	line_data->are = 0;
}

/**
 * Call-back function for list_add_ordered(); Performs lexicographical
 * comparison of two labels
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
int _parser_compare_labels(void* a, void* b) {
	Label* label = a;
	Label* label2 = b;
	return strcmp(label->label, label2->label);
}

/**
 * Call-back function for list_add_ordered(); Issues error message when a
 * duplicated label is declared.
 *
 * @param data
 *   Pointer to Label.
 */
void _parser_duplicated_label(void* data) {
	Label* label = data;
	error_set("Error", "Redeclaring label.", label->line);
}

/**
 * Free all of the parser variables.
 */
void parser_clean() {
	/* Free all lists. */
	list_destruct(parser_data.parser_symbols, &_parser_free_label);
	list_destruct(parser_data.parser_entry_symbols, &_parser_free_label);
	list_destruct(parser_data.data_list, &_parser_free_line_data);
	list_destruct(parser_data.commands_list, &_parser_free_line_data);

	parser_data.parser_symbols = NULL;
	parser_data.parser_entry_symbols = NULL;
	parser_data.data_list = NULL;
	parser_data.commands_list = NULL;

	current_char = NULL;

	parser_data.errors = 0;

	parser_data.IC = 0;
	parser_data.DC = 0;
}

/**
 * Check whether a line starts with a label.
 * Returns the label as a string. The string must be freed by the invoker.
 */
char* _parser_get_line_label(const char* line, int line_num) {
	int len = 0;
	char* label;
	const char* c = line;

	label = (char*)malloc(MaxLabelSize + 1);
	if (!label)
		error_fatal(ErrorMemoryAlloc);

	/* Iterate the first word in the line, until colon, space or end of line is
	 * found. */
	/* Label must start label. */
	if (!isalpha(*c)) {
		return NULL;
	}

	/* All other characters must be alpha-numeric. */
	while (isalnum(*c) && len < MaxLabelSize) {
		c++;
		len++;
	}

	if (line[len] == ':') {
		/* Potential label found. */

		strncpy(label, line, len);
		label[len] = '\0';

		/* Check if label name is a register name. */
		if ((strlen(label) == 2 && label[0] == 'r' && (label[1] - '0') >= 0 && (label[1] - '0') <= MaxRegisterNumber) || !strcmp(label, "PC") || !strcmp(label, "SP") || !strcmp(label, "PSW")) {
			error_set("Error", "Illegal label name, same as register.", line_num);
			return NULL;
		}

		return label;
	}

	return NULL;
}

/**
 * Extract number for .data line.
 *
 * @param word
 *   First character after .data
 * @param line_num
 *   The current line number.
 */
void extract_data_number(char* word, int const line_num) {
	int num_of_param = 0, num_of_comma = 0;
	long data_number;
	LineData* line_data;
	char* first_char;
	
	while (*word != '\0') {
		/* Find the beginning of the number */
		NextWord(word);
		if (*word == '\0' && num_of_param == 0)
			error_set("Error", "Data line contains no data.", line_num);

		if (*word == '\0')
			continue;
		
		/* Check for legal start of word. */
		if (*word != '-' && *word != '+' && !isdigit(*word)) {
			error_set("Error", "Data line contain illegal number", line_num);
			continue;
		}

		/* first_char will hold the first symbol in the number. */
		first_char = word;

		/* data_number will hold the extracted number. */
		data_number = 0;
		/* Extract the first digit. */
		if (isdigit(*word))
			data_number = *word - '0';

		word++;

		while (!IsBlank(*word) && *word != '\0' && *word != ',') {
			if (!isdigit(*word)) {
				error_set("Error", "data line contain illegal number", line_num);
				continue;
			} 
			else {
				/* Add the next digit to data number (In base 10). */
				data_number = 10 * data_number + *word - '0';
				/* Make sure we're still inside the 20bit limit. */
				if ((*first_char == '-' && data_number > -1 * MinDataNumber) || data_number > MaxDataNumber) {
					error_set("Error", "number is out of limit", line_num);
					while (!IsBlank(*word) && *word != '\0' && *word != ',')
						word++;

					continue;
				}
			}
			word++;
		}

		/* Convert to two's complement negative */
		if (*first_char == '-')
			data_number = Complement - data_number;

		/* Increment the data counter. */
		parser_data.DC++;
		/* Insert the line data to the list. */
		NewLineData(line_data);
		line_data->decimal_address = parser_data.DC;
		line_data->line_word.data = data_number;
		parser_data.data_list = list_append(parser_data.data_list, line_data);

		num_of_param++;

		NextWord(word);

		if (*word == ',') {
			word++;
			num_of_comma++;
		}
	}

	if (num_of_comma != (num_of_param - 1)) {
		error_set("Warning", "Data line contain spare comma at the end.", line_num);
	}
}

/**
 * Extract strings from .string lines.
 *
 * @param word
 *   The first character after ".string".
 * @param line_num
 *   The line number
 * @param line
 *   The line string.
 */
int extract_string(char* word, int const line_num, char* line) {
	LineData* line_data;
	char* current_char;
	NextWord(word);

	if (*word != '"') {
		error_set("Error", "String expected after .string", line_num);
		return 0;
	}

	/* Set current char to end of line. */
	current_char = line + strlen(line) - 1;

	/* Find last non whitespace character. */
	while (IsBlank(*current_char))
		current_char--;

	if (*(current_char) != '"' || current_char == word) {
		error_set("Error", "String expected after .string", line_num);
		return 0;
	}

	/* Create a data line for every character. */
	while (word + 1 < current_char) {
		parser_data.DC++;
		NewLineData(line_data);
		line_data->decimal_address = parser_data.DC;
		line_data->line_word.data = *(word + 1);
		parser_data.data_list = list_append(parser_data.data_list, line_data);

		word++;
	}

	/* "String delimiter". */
	parser_data.DC++;
	NewLineData(line_data);
	line_data->decimal_address = parser_data.DC;
	line_data->line_word.data = 0;
	parser_data.data_list = list_append(parser_data.data_list, line_data);

	return 0;
}

/**
 * Used to extract labels from within a line; For .extern and .entry lines.
 * Adds the labels to lists.
 *
 * @param word
 *   First character label.
 * @param current_char
 *   One character after the end of the label.
 * @param line_num
 *   The line number.
 * @param line
 *   The line.
 * @param line_type
 *   Specifies whether the line is .extern or .entry.
 */
void extract_label(char* word, char* current_char, int const line_num, char* line, LineType line_type) {
	Label* label;
	word = current_char;

	NextWord(word);

	if (!isalpha(*word)) {
		error_set("Error", "Not a legal label", line_num);
		return;
	}

	current_char = line + strlen(line) - 1;
	while (IsBlank(*current_char))
		current_char--;

	/* Check if label name is a register name. */
	if ((current_char - word == 1 && ((*word == 'r' && (*current_char - '0') >= 0 && (*current_char - '0') <= MaxRegisterNumber) || !strncmp(word, "PC", 2) || !strncmp(word, "SP", 2))) || (current_char - word == 2 && !strncmp(word, "PSW", 3))) {
		error_set("Error", "Illegal label name, same as reg.", line_num);
		return;
	}

	while (isalnum(*current_char))
		current_char--;

	if (current_char > word) {
		error_set("Error", "Label expected", line_num);
		return;
	}

	/* Insert the label to the labels list or to the entry labels list. */
	NewLabel(label);
	label->label = (char*)malloc(MaxLabelSize + 1);
	if (!label->label)
		error_fatal(ErrorMemoryAlloc);
	strcpy(label->label, word);

	switch (line_type) {
	case LineTypeEntry:
		/* Add the label the entry labels list. */
		label->line = line_num;
		parser_data.parser_entry_symbols = list_add_ordered(parser_data.parser_entry_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		break;

	case LineTypeExtern:
		/* Add the label the labels list. */
		label->label_type = LabelTypeExtern;
		label->line = 0;
		parser_data.parser_symbols = list_add_ordered(parser_data.parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		break;
	}
}

/**
 * Extracting relative addressing (Type 2).
 *
 * @param number
 *   String holding the number.
 * @param line_num
 *   The line number.
 *
 * @return
 *   The extracted number.
 */
long extract_number(char number[MaxLabelSize + 1], const int line_num) {
	long data_number;
	int k = 0;
	
	if (number[0] != '-' && number[0] != '+' && !isdigit(number[0])) {
		error_set("Error", "Illegal operand, expect number after.", line_num);
		return -1;
	}
	
	data_number = 0;
	if (isdigit(number[k]))
		data_number = number[k] - '0';
	k++;

	while (number[k] != '\0') {
		if (!isdigit(number[k])) {
			error_set("Error", "Illegal number.", line_num);
			return -1;
		} 
		
		else {
			data_number = 10 * data_number + number[k] - '0';
			if ((number[0] == '-' && data_number > -1 * MinDataNumber) || data_number > MaxDataNumber) {
				error_set("Error", "Number out of limit.", line_num);
				return -1;
			}
		}
		k++;
	}

	/* Convert negatives to two's complement. */
	if (number[0] == '-')
		return Complement - data_number;

	return data_number;
}

/**
 * Verify that the first operand is legal, while incrementing the current char
 * to the end of operand.
 *
 * @param operand
 *   String containing operand.
 * @param line_num
 *   The line number.
 *
 * @return
 *   1 when the operand are
 */
int extract_operand(char* operand, int line_num) {
	int i = 1;

	if (*current_char != '#' && !isalpha(*current_char)) {
		error_set("Error", "Illegal parameter.", line_num);
		return 0;
	}

	*operand = *current_char;
	current_char++;
	while (isalnum(*current_char) && i <= MaxLabelSize) {
		operand[i] = *current_char;
		current_char++;
		i++;
	}

	if (i == MaxLabelSize + 1) {
		error_set("Error", "Illegal operand.", line_num);
		return 0;
	}

	operand[i] = '\0';

	/* Assuming no white chars at middle of operand. */
	if ((*current_char != '{' && *current_char != ',' && *current_char != '\0' && !IsBlank(*current_char)) || (operand[0] == '#' && *current_char == '{')) {
		error_set("Error", "Illegal parameter.", line_num);
		return 0;
	}
	return 1;
}

/**
 * Used for extracting the relative addressing value, while incrementing
 * current_char the the end of the operand.
 *
 * @param operand_offset
 *   The first character after the opening parenthesis.
 * @param line_num
 *   The line number.
 *
 * @return
 *   1 if operand was found.
 */
int extract_operand_offset(char* operand_offset, int line_num) {
	int i = 0;

	/* Extracting offset for first parameter if any. */
	current_char++;

	if (*current_char=='+' || *current_char=='-') {
		operand_offset[0] = *current_char;
		current_char++;
		i++;
 	}

	while (isalnum(*current_char) && i <= MaxLabelSize) {
		operand_offset[i] = *current_char;
		current_char++;
		i++;
	}

	if (i == MaxLabelSize + 1 || *current_char != '}') {
		error_set("Error", "Illegal operand.", line_num);
		return 0;
	}

	operand_offset[i] = '\0';
	current_char++;
	return 1;
}

/**
 * @param line_data
 * @param operand
 * @param operand_offset
 * @param work_on_src
 */
int update_operand(LineData* line_data, char* operand,char* operand_offset, int work_on_src) {
	/* Index address. */

	if (*operand_offset != '\0') {
		if (work_on_src) {
			line_data->line_word.inst.src_address = 2;
			if (*operand_offset == 'r' && operand_offset[1] >= '0' && operand_offset[1] <= MaxRegisterNumber + '0' && strlen(operand_offset) == 2)
				line_data->line_word.inst.src_reg = operand_offset[1] - '0';
		}
		else {
			line_data->line_word.inst.dest_address = 2;
			if (*operand_offset == 'r' && operand_offset[1] >= '0' && operand_offset[1] <= MaxRegisterNumber + '0' && strlen(operand_offset) == 2)
				line_data->line_word.inst.dest_reg = operand_offset[1] - '0';
		}
		return 0;
	}

	/* Register address. */
	if (*operand == 'r' && operand[1] >= '0' && operand[1] <= MaxRegisterNumber + '0' && strlen(operand) == 2) {
		if (work_on_src) {
			line_data->line_word.inst.src_address = 3;
			line_data->line_word.inst.src_reg = operand[1] - '0';
		}
		else {
			line_data->line_word.inst.dest_address = 3;
			line_data->line_word.inst.dest_reg = operand[1] - '0';
		}
		return 0;
	}

	/* Direct address. */
	if (*operand != '#') {
		if (work_on_src)
			line_data->line_word.inst.src_address = 1;
		else 
			line_data->line_word.inst.dest_address = 1;
		return 0;
	}

	/* For immediate address it is already 0. */
	return 0;
}

int add_operand_lines (char *operand, char *operand_offset, int work_on_src, int i, int line_num, int addr) {
	LineData* line_data = NULL;
	switch (addr) {
		case 0: 
			if ((!commands[i].src_imidiate_address && work_on_src) || (!commands[i].dest_imidiate_address && !work_on_src)) {
				error_set("Error", "Illegal address.", line_num);
				return 0;
			}

			parser_data.IC++;
			NewLineData(line_data);
			line_data->decimal_address = parser_data.IC;
			line_data->are = 'a';
			parser_data.commands_list = list_append(parser_data.commands_list, line_data);

			if((line_data->line_word.data = extract_number(&operand[1], line_num)) == -1)
				return 0;
			break;

		case 1:
			if ((!commands[i].src_direct_address && work_on_src) || (!commands[i].dest_direct_address && !work_on_src)) {
				error_set("Error", "Illegal address.", line_num);
				return 0;
			}

			parser_data.IC++;
			NewLineData(line_data);
			line_data->decimal_address = parser_data.IC;
			line_data->label_to_extract = (char*)malloc(strlen(operand) + 1);
			strcpy(line_data->label_to_extract, operand);
			parser_data.commands_list = list_append(parser_data.commands_list, line_data);
			break;

		case 2:
			if ((!commands[i].src_index_address && work_on_src) || (!commands[i].dest_index_address && !work_on_src)) {
				error_set("Error", "Illegal address.", line_num);
				return 0;
			}

			parser_data.IC++;
			NewLineData(line_data);
			line_data->decimal_address = parser_data.IC;
			line_data->label_to_extract = (char*)malloc(strlen(operand) + 1);
			strcpy(line_data->label_to_extract, operand);			
			parser_data.commands_list = list_append(parser_data.commands_list, line_data);
		
			/* Adding offset address. */

			if (isdigit(*operand_offset) || *operand_offset=='+' || *operand_offset=='-') {
				parser_data.IC++;
				NewLineData(line_data);
				line_data->decimal_address = parser_data.IC;
				line_data->are = 'a';
				if((line_data->line_word.data = extract_number(operand_offset, line_num)) == -1) {
					error_set("Error", "Illegal address.", line_num);
					return 0;
				}
				parser_data.commands_list = list_append(parser_data.commands_list, line_data);
			}

			else if (!(*operand_offset == 'r' && strlen(operand_offset) == 2 && *(operand_offset + 1) >= '0' && *(operand_offset + 1) <= MaxRegisterNumber + '0')) {
				parser_data.IC++;
				NewLineData(line_data);
				line_data->decimal_address = parser_data.IC;
				line_data->label_to_extract = (char*)malloc(strlen(operand_offset) + 1);
				strcpy(line_data->label_to_extract, operand_offset);
				parser_data.commands_list = list_append(parser_data.commands_list, line_data);
			}
			break;

		case 3:
			if  ((!commands[i].src_direct_reg_address && work_on_src) || (!commands[i].dest_direct_reg_address && !work_on_src)) {
				error_set("Error", "Illegal address.", line_num);
				return 0;
			}
			break;
		}

	return 1;
}

/**
 * Does the initial parsing of the assembly file.
 *
 * @return
 *  0 when any errors where introduced during the parsing.
 */
int parse() {
	/* Beginning of line*/
	char* line;
	/* Operands. */
	char operand1[MaxLabelSize + 1], operand2[MaxLabelSize + 1];
	/* Operands for type 2 addressing */
	char operand1_offset[MaxLabelSize + 1], operand2_offset[MaxLabelSize + 1];
	/* Line label. */
	Label* label = NULL;
	/* Holds all of the about the line being read. */
	LineData* line_data = NULL;
	char *word;
	char command_type[MaxRegisterNumber];
	/* Used for destination address only. */
	int address;
	int line_num = 0;
	int i, j;

	printf("Parsing %s.\n", reader_get_file_name(ReaderFileExtension));
	while ((line = reader_get_line())) {
		line_num++;

		/* Commented-out line. */
		if (*line == ';')
			continue;

		/* Create new label. */
		NewLabel(label);
		label->label = _parser_get_line_label(line, line_num);
		word = line;

		/* Finding first word. */
		NextWord(word);

		/* Empty line. */
		if (*word == '\0') {
			free (label);
			continue;
		}

		/* Find beginning of next word after label. */
		if (label->label) {
			word = line + strlen(label->label) + 1;
			NextWord(word);
		}

		if (*word == '\0' && label->label) {
			/* Assuming every label declaration must follow commands or
			 * declaration of data. */
			error_set("Error", "Label with no command", line_num);
			free (label);
			continue;
		}

		/* Find end of command. */
		current_char = word + 1;
		while (!IsBlank(*current_char) && *current_char != '/' && *current_char != '\0')
			current_char++;

		/* Extracting label of .string or .data lines. */
		if (((!strncmp(word, ".data", 5) && (current_char - word) == 5 && *current_char != '/')
		    || (!strncmp(word, ".string", 7) && (current_char - word) == 7 && *current_char != '/'))
		    && label->label) {

			/* TODO: Explain +1 */
			label->line = parser_data.DC + 1;
			label->label_type = LabelTypeData;
			/* Add the label to the labels list. */
			parser_data.parser_symbols = list_add_ordered(parser_data.parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		}

		/* Data line. */
		if (!strncmp(word, ".data", 5) && (current_char - word) == 5 && *current_char != '/') {
			extract_data_number(current_char, line_num);
			continue;
		}

		/* String line. */
		if (!strncmp(word, ".string", 7) && (current_char - word) == 7 && *current_char != '/') {
			extract_string(current_char, line_num, line);
			continue;
		}

		/* Entry label declaration line. */
		if (!strncmp(word, ".entry", 6) && (current_char - word) == 6 && *current_char != '/') {
			extract_label(word, current_char, line_num, line, LineTypeEntry);
			continue;
		}

		/* External label declaration line. */
		if (!strncmp(word, ".extern", 7) && (current_char - word) == 7 && *current_char != '/') {
			extract_label(word, current_char, line_num, line, LineTypeExtern);
			continue;
		}

		/* Insert a instruction line to the line data list. */
		parser_data.IC++;
		NewLineData(line_data);
		line_data->decimal_address = parser_data.IC;
		parser_data.commands_list = list_append(parser_data.commands_list, line_data);
		/* 'a' is for Absolute. */
		line_data->are = 'a';
		line_data->is_instruction = 1;

		/* Command line. */
		if(label->label) {
			label->line = parser_data.IC;
			label->label_type = LabelTypeCommand;
			parser_data.parser_symbols = list_add_ordered(parser_data.parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		}

		if (strlen(line) > MaxLineSize) {
			error_set("Error", "Line length exceeding 80 characters.", line_num);
			continue;
		}

		/* Find command code. */
		for (i = 0; i < CommandsAmount && strncmp(word, commands[i].command, current_char - word); i++);
		if (i == CommandsAmount) {
			error_set("Error", "Unknown command.", line_num);
			continue;
		}

		/* Set command code on the line. */
		line_data->line_word.inst.opcode = i;
		/* Find '/0' or /1 , assuming /0 is not followed by other options. */
		NextWord(current_char);
		command_type[0] = *current_char;
		if (*current_char != '\0') {
			current_char++;
			NextWord(current_char);
			command_type[1] = *current_char;
			command_type[2] = '\0';
		}
		if (!(command_type[0] == '/' && (command_type[1] == '0' || command_type[1] == '1'))) {
			error_set("Error", "Type expected after command.", line_num);
			continue;
		}

		if (*current_char != '\0')
			current_char++;

		/* Extract first operand. */
		if (command_type[1] == '1') {
			for (j = 0; j < 4; j++) {
				NextWord(current_char);
				command_type[2 + j] = *current_char;
				command_type[2 + j + 1] = '\0';
				if (*current_char != '\0')
					current_char++;
			}

			if (!(command_type[0] == '/' &&
				command_type[2] == '/' &&
				command_type[4] == '/' &&
				(command_type[5] == '0' || command_type[5] == '1') &&
				(command_type[3] == '0' || command_type[3] == '1'))) {
				error_set("Error", "Illegal command type", line_num);
				continue;
			}

			line_data->line_word.inst.type = command_type[1] - '0';
			line_data->line_word.inst.comb = command_type[5] - '0' + 2 * (command_type[3] - '0');
		}

		/* Verify blank after type. */
		if (!IsBlank(*current_char) && *current_char != '\0') {
			error_set("Error", "Blank char is required after command.", line_num);
			continue;
		}

		/* Find operands. */
		operand1[0] = '\0';
		operand2[0] = '\0';
		operand1_offset[0] = '\0';
		operand2_offset[0] = '\0';
		NextWord(current_char);

		if (*current_char!='\0') {
			if (!extract_operand(operand1, line_num))
				continue;

			if (*current_char == '{')
				if (!extract_operand_offset(operand1_offset, line_num))
					continue;
		}

		NextWord(current_char);

		/* Extract other operands. */
		if (*current_char == ',') {
			current_char++;
			NextWord(current_char);
			if (!extract_operand(operand2, line_num))
				continue;

			if (*current_char == '{')
				if (!extract_operand_offset(operand2_offset, line_num))
					continue;
		}

		NextWord(current_char);

		if (*current_char != '\0' || (*operand1 == '#' && *operand1_offset != '\0') || (*operand2 == '#' && *operand2_offset != '\0')) {
			error_set("Error", "Illegal parameters.", line_num);
			continue;
		}

		/* No operand required. */
		if (!commands[i].src_operand && !commands[i].dest_operand && *operand1 != '\0') {
			error_set("Error", "No operands required", line_num);
			continue;
		}

		/* One operand required. */
		if (!commands[i].src_operand && commands[i].dest_operand && (*operand1 == '\0' || *operand2 != '\0')) {
			error_set("Error", "Exactly one operand required.", line_num);
			continue;
		}

		/* Two operands required. */
		if (commands[i].src_operand && commands[i].dest_operand &&  (*operand1 == '\0' || *operand2 == '\0')) {
			error_set("Error", "Two operands required.", line_num);
			continue;
		}

		/* Handling addressing. */
		/* Two operands exist. */
		if (*operand2 != '\0') {
			update_operand(line_data, operand1, operand1_offset, 1);
			update_operand(line_data, operand2, operand2_offset, 0);
			address = line_data->line_word.inst.dest_address;

			if (!add_operand_lines(operand1, operand1_offset, 1, i, line_num, line_data->line_word.inst.src_address))
				continue;

			if (!add_operand_lines(operand2, operand2_offset, 0, i, line_num, address))
				continue;
		}

		/* When one operand exists. */
		if (*operand1 != '\0' && *operand2 == '\0') {
			update_operand(line_data, operand1, operand1_offset, 0);
			if (!add_operand_lines(operand1, operand1_offset, 0, i, line_num, line_data->line_word.inst.dest_address))
				continue;
		}
	}

	return !parser_data.errors;
}
