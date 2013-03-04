#include "error.h"
#include "list.h"
#include "parser.h"
#include "reader.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ParserData parser_data;
LineData* line_data;

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

char* word_end;

void print_list(List list){
	Label *label;
	ListNodePtr p = list;
	if (!p) {
		return;
	}
	while (p->next) {
		label = p->data;
		printf("%s:\t%d\t%d\n", label->label, label->label_type, label->line);
		p = p->next;
	}
}

void print_data_list(List list) {
	LineData* line_data;
	ListNodePtr p = list;
	if (!p) {
		return;
	}
	while (p) {
		line_data = p->data;

		printf("\tAddress:%d\tLabel: %s\tData: %ld\n", line_data->decimal_address, line_data->label_to_extract, line_data->line_word.data);

		p = p->next;
	}
}

ParserData* parser_parse() {
	char* line, operand1[MAX_LABEL_SIZE + 1], operand1_offset[MAX_LABEL_SIZE + 1], operand2[MAX_LABEL_SIZE + 1], operand2_offset[MAX_LABEL_SIZE + 1];
	Label* label;

	char *word, command_type[7];
	/* "address" is used for destination address only. */
	int line_num = 0, i, j, address;

	printf("Parsing %s.\n", reader_get_file_name(ReaderFileExtension));
	while ((line = reader_get_line())) {
		line_num++;

		/* Commented-out line. */
		if (*line == ';')
			continue;

		label = New(Label);
		label->label = parser_get_label(line, line_num);
		word = line;

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
		word_end = word + 1;
		while (!IsBlank(*word_end) && *word_end != '/' && *word_end != '\0')
			word_end++;

		if (((!strncmp(word, ".data", 5) && (word_end - word) == 5 && *word_end != '/')
		    || (!strncmp(word, ".string", 7) && (word_end - word) == 7 && *word_end != '/'))
		    && label->label) {

			label->line = parser_data.DC + 1;
			label->label_type = LABEL_TYPE_DATA;
			parser_data.parser_symbols = list_add_ordered(parser_data.parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		}			

		/* Data line. */
		if (!strncmp(word, ".data", 5) && (word_end - word) == 5 && *word_end != '/') {
			extract_data_number(word_end, line_num);
			continue;
		}

		/* String line. */
		if (!strncmp(word, ".string", 7) && (word_end - word) == 7 && *word_end != '/') {
			extract_string(word_end, line_num, line);
			continue;
		}

		/* Entry label declaration line. */
		if (!strncmp(word, ".entry", 6) && (word_end - word) == 6 && *word_end != '/') {
			extract_label(word, word_end, line_num, line, LINE_TYPE_ENTRY);
			continue;
		}

		/* External label declaration line. */
		if (!strncmp(word, ".extern", 7) && (word_end - word) == 7 && *word_end != '/') {
			extract_label(word, word_end, line_num, line, LINE_TYPE_EXTERN);
			continue;
		}

		parser_data.IC++;
		line_data = New(LineData);
		line_data->decimal_address = parser_data.IC;
		line_data->line_word.data = 0;
		line_data->label_to_extract = NULL;
		parser_data.commands_list = list_append(parser_data.commands_list, line_data);
		line_data->are = 'a';

		/* Command line. */
		if(label->label) {
			label->line = parser_data.IC;
			label->label_type = LABEL_TYPE_COMMAND;
			parser_data.parser_symbols = list_add_ordered(parser_data.parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		}
		
		if (strlen(line) > 80) {
			error_set("Error", "Line length exceeding 80 characters.", line_num);
			continue;
		}

		for (i = 0; i < 16 && strncmp(word, commands[i].command, word_end - word); i++);
		if (i == 16) {
			error_set("Error", "Unknown command.", line_num);
			continue;
		}

		line_data->line_word.inst.opcode = i;
		/* Find '/0' or /1 , assuming /0 is not followed by other options. */
		NextWord(word_end);
		command_type[0] = *word_end;
		if (*word_end != '\0') {
			word_end++;
			NextWord(word_end);
			command_type[1] = *word_end;
			command_type[2] = '\0';
		}
		if (!(command_type[0] == '/' && (command_type[1] == '0' || command_type[1] == '1'))) {
			error_set("Error", "Type expected after command.", line_num);
			continue;
		}

		if (*word_end != '\0')
			word_end++;

		if (command_type[1] == '1') {
			for (j = 0; j < 4; j++) {
				NextWord(word_end);
				command_type[2 + j] = *word_end;
				command_type[2 + j + 1] = '\0';
				if (*word_end != '\0')
					word_end++;
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
		if (!IsBlank(*word_end) && *word_end != '\0') {
			error_set("Error", "Blank char is required after command.", line_num);
			continue;
		}

		/* Find operands. */
		operand1[0] = '\0';
		operand2[0] = '\0';
		operand1_offset[0] = '\0';
		operand2_offset[0] = '\0';
		NextWord(word_end);

		if (*word_end!='\0') {
			if (!extract_operand(operand1, i, line_num))
				continue;

			if (*word_end == '{')
				if (!extract_operand_offset(operand1_offset, i, line_num))
					continue;
		}

		NextWord(word_end);
		
		if (*word_end == ',') {
			word_end++;
			NextWord(word_end);
			if (!extract_operand(operand2, i, line_num))
				continue;

			if (*word_end == '{')
				if (!extract_operand_offset(operand2_offset, i, line_num))
					continue;
		}

		NextWord(word_end);

		if (*word_end != '\0' || (*operand1 == '#' && *operand1_offset != '\0') || (*operand2 == '#' && *operand2_offset != '\0')) {
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
			update_operand(operand1, operand1_offset, 1);
			update_operand(operand2, operand2_offset, 0);
			address = line_data->line_word.inst.dest_address;

			if (!add_operand_lines(operand1, operand1_offset, 1, i, line_num, line_data->line_word.inst.src_address))
				continue;

			if (!add_operand_lines(operand2, operand2_offset, 0, i, line_num, address))
				continue;
		}

		/* when one operand exists*/
		if (*operand1 != '\0' && *operand2 == '\0') {
			update_operand(operand1, operand1_offset, 0);
			if (!add_operand_lines(operand1, operand1_offset, 0, i, line_num, line_data->line_word.inst.dest_address))
				continue;
		}
	}

	return &parser_data;
}

void _parser_free_label(void* data) {
	Label* label = data;
	free(label->label);
}

void _parser_free_line_data(void* data) {
	LineData* line_data = data;
	free(line_data->label_to_extract);
}

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

	line_data = NULL;

	word_end = NULL;

	parser_data.parser_errors = 0;

	parser_data.IC = 0;
	parser_data.DC = 0;
}

char* parser_get_label(const char* line, int line_num) {
	int len = 0;
	char* label;
	const char* c = line;

	label = (char*)malloc(MAX_LABEL_SIZE + 1);
	if (!label)
		error_fatal(ErrorMemoryAlloc);

	/* Iterate the first word in the line, until colon, space or end of line is
	 * found. */
	if (!isalpha(*c)) {
		return NULL;
	}

	while (isalnum(*c) && len < MAX_LABEL_SIZE) {
		c++;
		len++;
	}

	if (line[len] == ':') {
		strncpy(label, line, len);
		label[len] = '\0';

		/* check if label name is a reg name*/
		if ((strlen(label) == 2 && label[0] == 'r' && (label[1] - '0') >= 0 && (label[1] - '0') <= 7) || !strcmp(label, "PC") || !strcmp(label, "SP") || !strcmp(label, "PSW")) {
			error_set("Error", "Illegal label name, same as reg.", line_num);
			return NULL;
		}

		return label;
	}

	return NULL;
}

int _parser_compare_labels(void* a, void* b) {
	Label* label = a;
	Label* label2 = b;
	return strcmp(label->label, label2->label);
}

void _parser_duplicated_label(void* data) {
	Label* label = data;
	error_set("Error", "Redeclaring label.", label->line);
}

void extract_data_number(char* word, int const line_num) {
	int num_of_param = 0, num_of_comma = 0;
	long data_number;
	LineData* line_data;
	
	while (*word != '\0') {
		NextWord(word);
		if (*word == '\0' && num_of_param == 0)
			error_set("Warning", "Data line contains no data.", line_num);

		if (*word == '\0')
			continue;
		
		if (*word != '-' && *word != '+' && !isdigit(*word)) {
			error_set("Error", "Data line contain illegal number", line_num);
			continue;
		}

		word++;
		data_number = 0;
		if (isdigit(*word))
			data_number = *word - '0';

		while (!IsBlank(*word_end) && *word_end != '\0' && *word_end != ',') {
			if (!isdigit(*word_end)) {
				error_set("Error", "data line contain illegal number", line_num);
				continue;
			} 
			else {
				/* @TODO: Explain. */
				data_number = 10 * data_number + *word_end - '0';
				if ((*word == '-' && data_number > -1 * MIN_DATA_NUMBER) || data_number > MAX_DATA_NUMBER) {
					error_set("Error", "number is out of limit", line_num);
					while (!IsBlank(*word_end) && *word_end != '\0' && *word_end != ',')
						word_end++;

					continue;
				}
			}
			word_end++;
		}

		if (*word == '-')
			data_number = MINUS - data_number;

		parser_data.DC++;
		line_data = New(LineData);
		line_data->are = 0;
		line_data->label_to_extract = NULL;
		line_data->decimal_address = parser_data.DC;
		line_data->line_word.data = data_number;

		parser_data.data_list = list_append(parser_data.data_list, line_data);

		num_of_param++;

		NextWord(word_end);

		if (*word_end == ',') {
			word_end++;
			num_of_comma++;
		}
	}
	if (num_of_comma != (num_of_param - 1)) {
		error_set("Warning", "Data line contain spare comma at the end.", line_num);
	}
}

int extract_string(char* word, int const line_num, char* line) {
	LineData* line_data;
	char * word_end;
	NextWord(word);

	if (*word != '"') {
		error_set("Error", "String expected after .string", line_num);
		return 0;
	}

	word_end = line + strlen(line) - 1;

	while (IsBlank(*word_end))
		word_end--;

	if (*(word_end) != '"' || word_end == word) {
		error_set("Error", "String expected after .string", line_num);
		return 0;
	}

	while (word + 1 < word_end) {
		parser_data.DC++;
		line_data = New(LineData);
		line_data->are = 0;
		line_data->label_to_extract = NULL;
		line_data->decimal_address = parser_data.DC;
		line_data->line_word.data = *(word + 1);
		parser_data.data_list = list_append(parser_data.data_list, line_data);

		word++;
	}

	parser_data.DC++;
	line_data = New(LineData);
	line_data->are = 0;
	line_data->label_to_extract = NULL;
	line_data->decimal_address = parser_data.DC;
	line_data->line_word.data = 0;
	parser_data.data_list = list_append(parser_data.data_list, line_data);

	return 0;
}

void extract_label(char * word, char *word_end, int const line_num, char * line, LineType line_type) {
	Label* label;
	word = word_end;

	NextWord(word);

	if (!isalpha(*word)) {
		error_set("Error", "Not a legal label", line_num);
		return;
	}

	word_end = line + strlen(line) - 1;
	while (IsBlank(*word_end))
		word_end--;

	/* Check if label name is a register name. */
	if ((word_end - word == 1 && ((*word == 'r' && (*word_end - '0') >= 0 && (*word_end - '0') <= 7) || !strncmp(word, "PC", 2) || !strncmp(word, "SP", 2))) || (word_end - word == 2 && !strncmp(word, "PSW", 3))) {
		error_set("Error", "Illegal label name, same as reg.", line_num);
		return;
	}

	while (isalnum(*word_end))
		word_end--;

	if (word_end > word) {
		error_set("Error", "Label expected", line_num);
		return;
	}

	/* Insert the label to the external labels list or to the entry labels list.
	 */
	label = New(Label);

	label->label = (char*)malloc(MAX_LABEL_SIZE + 1);
	if (!label->label)
		error_fatal(ErrorMemoryAlloc);
	strcpy(label->label, word);

	switch (line_type) {
	case LINE_TYPE_ENTRY:
		label->line = line_num;
		parser_data.parser_entry_symbols = list_add_ordered(parser_data.parser_entry_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		break;

	case LINE_TYPE_EXTERN:
		label->label_type = LABEL_TYPE_EXTERN;
		label->line = 0;
		parser_data.parser_symbols = list_add_ordered(parser_data.parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		break;
	}
}

long extract_number(char number[MAX_LABEL_SIZE + 1], const int line_num) {
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
			if ((number[0] == '-' && data_number > -1 * MIN_DATA_NUMBER) || data_number > MAX_DATA_NUMBER) {
				error_set("Error", "Number out of limit.", line_num);
				return -1;
			}
		}
		k++;
	}

	if (number[0] == '-')

		return MINUS-data_number;

	return data_number;
}

int extract_operand(char *operand,int i,int line_num ) {
	int j;

	if (*word_end != '#' && !isalpha(*word_end)) {
		error_set("Error", "Illegal parameter.", line_num);
		return 0;
	}

	*operand = *word_end;
	word_end++;
	j = 1;
	while (isalnum(*word_end) && j <= MAX_LABEL_SIZE) {
		operand[j] = *word_end;
		word_end++;
		j++;
	}

	if (j == MAX_LABEL_SIZE + 1) {
		error_set("Error", "Illegal operand.", line_num);
		return 0;
	}

	operand[j] = '\0';

	/* Assuming no white chars at middle of operand. */
	if ((*word_end != '{' && *word_end != ',' && *word_end != '\0' && !IsBlank(*word_end)) || (operand[0] == '#' && *word_end == '{')) {
		error_set("Error", "Illegal parameter.", line_num);
		return 0;
	}
	return 1;
}

int extract_operand_offset(char * operand_offset,int i,int line_num) {
	int j;

	/* Extracting offset for first parameter if any. */
	word_end++;

	j = 0;

	if (*word_end=='+' || *word_end=='-') {
		operand_offset[0] = *word_end;
		word_end++;
		j++;
 	}

	while (isalnum(*word_end) && j <= MAX_LABEL_SIZE) {
		operand_offset[j] = *word_end;
		word_end++;
		j++;
	}

	if (j == MAX_LABEL_SIZE + 1 || *word_end != '}') {
		error_set("Error", "Illegal operand.", line_num);
		return 0;
	}

	operand_offset[j] = '\0';
	word_end++;
	return 1;
}

int update_operand(char *operand,char *operand_offset,int work_on_src) {
	/* Index address. */

	if (*operand_offset != '\0') {
		if (work_on_src) {
			line_data->line_word.inst.src_address = 2;
			if (*operand_offset == 'r' && operand_offset[1] >= '0' && operand_offset[1] <= '7' && strlen(operand_offset) == 2)
				line_data->line_word.inst.src_reg = operand_offset[1] - '0';
		}
		else {
			line_data->line_word.inst.dest_address = 2;
			if (*operand_offset == 'r' && operand_offset[1] >= '0' && operand_offset[1] <= '7' && strlen(operand_offset) == 2)
				line_data->line_word.inst.dest_reg = operand_offset[1] - '0';
		}
		return 0;
	}

	/* Register address. */
	if (*operand == 'r' && operand[1] >= '0' && operand[1] <= '7' && strlen(operand) == 2) {
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

	switch (addr) {
		case 0: 
			if ((!commands[i].src_imidiate_address && work_on_src) || (!commands[i].dest_imidiate_address && !work_on_src)) {
				error_set("Error", "Illegal address.", line_num);
				return 0;
			}

			parser_data.IC++;
			line_data = New(LineData);
			line_data->decimal_address = parser_data.IC;
			line_data->label_to_extract = NULL;
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
			line_data = New(LineData);
			line_data->decimal_address = parser_data.IC;
			line_data->line_word.data = 0;
			line_data->are = 0;
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
			line_data = New(LineData);
			line_data->decimal_address = parser_data.IC;
			line_data->line_word.data = 0;
			line_data->are = 0;
			line_data->label_to_extract = (char*)malloc(strlen(operand) + 1);
			strcpy(line_data->label_to_extract, operand);			
			parser_data.commands_list = list_append(parser_data.commands_list, line_data);
		
			/* Adding offset address. */

			if (isdigit(*operand_offset) || *operand_offset=='+' || *operand_offset=='-') {
				parser_data.IC++;
				line_data = New(LineData);
				line_data->decimal_address = parser_data.IC;
				line_data->label_to_extract = NULL;
				line_data->are = 'a';
				if((line_data->line_word.data = extract_number(operand_offset, line_num)) == -1) {
					error_set("Error", "Illegal address.", line_num);
					return 0;
				}
				parser_data.commands_list = list_append(parser_data.commands_list, line_data);
			}

			else if (!(*operand_offset == 'r' && strlen(operand_offset) == 2 && *(operand_offset + 1) >= '0' && *(operand_offset + 1) <= '7')) {
				parser_data.IC++;
				line_data = New(LineData);
				line_data->decimal_address = parser_data.IC;
				line_data->are = 0;
				line_data->line_word.data = 0;
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
