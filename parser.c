#include "char.h"
#include "error.h"
#include "list.h"
#include "parser.h"
#include "reader.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List parser_symbols;
List parser_entry_symbols;
/* List for data. */
List data_list;
/* List for commands. */
List commands_list;

LineData* line_data;

/* Command and data counters. */
int IC = 0, DC = 0;

FILE* output_files[3];

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

char* end_of_word;

void parser_parse() {

	char* line, operand1[MAX_LABEL_SIZE + 1], operand1_offset[MAX_LABEL_SIZE + 1], operand2[MAX_LABEL_SIZE + 1], operand2_offset[MAX_LABEL_SIZE + 1];
	Label* label;
	
	char *begin_of_word, command_type[7];
	/* "address" is used for destination address only. */
	int line_num = 0, i, j, address;

	while ((line = reader_get_line())) {
		line_num++;

		/* Commented-out line. */
		if (*line == ';')
			continue;

		label = New(Label);
		label->label = parser_get_label(line, line_num);
		begin_of_word = line;

		NextWord(begin_of_word);

		/* Empty line. */
		if (*begin_of_word == '\0') {
			free (label);
			continue;
		}

		/* Find beginning of next word after label. */
		if (label->label) {
			begin_of_word = line + strlen(label->label) + 1;
			NextWord(begin_of_word);
		}

		if (*begin_of_word == '\0' && label->label) {
			/* Assuming every label declaration must follow commands or
			 * declaration of data. */
			error_set("Error", "Label with no command", line_num);
			free (label);
			continue;
		}

		/* Find end of command. */
		end_of_word = begin_of_word + 1;
		while (!char_isblank(*end_of_word) && *end_of_word != '/' && *end_of_word != '\0')
			end_of_word++;

		if (((!strncmp(begin_of_word, ".data", 5) && (end_of_word - begin_of_word) == 5 && *end_of_word != '/')
		    || (!strncmp(begin_of_word, ".string", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/'))
		    && label->label) {

			label->line = DC + 1;
			label->label_type = LABEL_TYPE_DATA;
			parser_symbols = list_add_ordered(parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		}			

		/* Data line. */
		if (!strncmp(begin_of_word, ".data", 5) && (end_of_word - begin_of_word) == 5 && *end_of_word != '/') {
			extract_data_number(end_of_word, line_num);
			continue;
		}

		/* String line. */
		if (!strncmp(begin_of_word, ".string", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/') {
			extract_string(end_of_word, line_num, line);
			continue;
		}

		/* Entry label declaration line. */
		if (!strncmp(begin_of_word, ".entry", 6) && (end_of_word - begin_of_word) == 6 && *end_of_word != '/') {
			extract_label(begin_of_word, end_of_word, line_num, line, LINE_TYPE_ENTRY);
			continue;
		}

		/* External label declaration line. */
		if (!strncmp(begin_of_word, ".extern", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/') {
			extract_label(begin_of_word, end_of_word, line_num, line, LINE_TYPE_EXTERN);
			continue;
		}

		IC++;
		line_data = New(LineData);
		line_data->decimal_address = IC;
		line_data->line_word.data = 0;
		line_data->label_to_extract = NULL;
		commands_list = list_append(commands_list, line_data);

		/* Command line. */
		if(label->label) {
			label->line = IC;
			label->label_type = LABEL_TYPE_COMMAND;
			parser_symbols = list_add_ordered(parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		}
		
		if (strlen(line) > 80) {
			error_set("Error", "Line length exceeding 80 characters.", line_num);
			continue;
		}

		for (i = 0; i < 16 && strncmp(begin_of_word, commands[i].command, end_of_word - begin_of_word); i++);
		if (i == 16) {
			error_set("Error", "Unknown command.", line_num);
			continue;
		}

		line_data->line_word.inst.opcode = i;
		/* Find '/0' or /1 , assuming /0 is not followed by other options. */
		NextWord(end_of_word);
		command_type[0] = *end_of_word;
		if (*end_of_word != '\0') {
			end_of_word++;
			NextWord(end_of_word);
			command_type[1] = *end_of_word;
			command_type[2] = '\0';
		}
		if (!(command_type[0] == '/' && (command_type[1] == '0' || command_type[1] == '1'))) {
			error_set("Error", "Type expected after command.", line_num);
			continue;
		}

		if (*end_of_word != '\0')
			end_of_word++;

		if (command_type[1] == '1') {
			for (j = 0; j < 4; j++) {
				NextWord(end_of_word);
				command_type[2 + j] = *end_of_word;
				command_type[2 + j + 1] = '\0';
				if (*end_of_word != '\0')
					end_of_word++;
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
		if (!char_isblank(*end_of_word) && *end_of_word != '\0') {
			error_set("Error", "Blank char is required after command.", line_num);
			continue;
		}

		/* Find operands. */
		operand1[0] = '\0';
		operand2[0] = '\0';
		operand1_offset[0] = '\0';
		operand2_offset[0] = '\0';
		NextWord(end_of_word);

		if (*end_of_word!='\0') {
			if (!extract_operand(operand1, i, line_num))
				continue;

			if (*end_of_word == '{')
				if (!extract_operand_offset(operand1_offset, i, line_num))
					continue;
		}

		NextWord(end_of_word);
		
		if (*end_of_word == ',') {
			end_of_word++;
			NextWord(end_of_word);
			if (!extract_operand(operand2, i, line_num))
				continue;

			if (*end_of_word == '{')
				if (!extract_operand_offset(operand2_offset, i, line_num))
					continue;
		}

		NextWord(end_of_word);

		if (*end_of_word != '\0' || (*operand1 == '#' && *operand1_offset != '\0') || (*operand2 == '#' && *operand2_offset != '\0')) {
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
}

void parser_translate_commands() {
	OpenFile(output_files[EXT_FILE], "ext");
	OpenFile(output_files[OB_FILE], "ob");

	list_foreach(commands_list, &_parser_translate_command);
	list_foreach(data_list, &_parser_translate_data);

	fclose(output_files[EXT_FILE]);
	fclose(output_files[OB_FILE]);
}

void _parser_translate_line(LineData* line_data, unsigned int extra_address_offset) {
	Label* label_found = NULL;
	Label* dummy_label;

	if (line_data->label_to_extract) {
		dummy_label = New(Label);
		dummy_label->label = line_data->label_to_extract;
		label_found = list_find_item(parser_symbols, dummy_label, &_parser_compare_labels);
		free(dummy_label);

		if (!label_found)
			fprintf(stderr, "Error: Label \"%s\" not found\n", line_data->label_to_extract);
		else {
			line_data->line_word.data = label_found->line + LINE_OFFSET - 1 + extra_address_offset;
			if (label_found->label_type == LABEL_TYPE_DATA)
				line_data->line_word.data += IC;
		}
	}

	fprintf(output_files[OB_FILE], "%ld\t", to_base4(line_data->decimal_address + LINE_OFFSET - 1 + extra_address_offset));
	fprintf(output_files[OB_FILE], "%010ld\t", to_base4(line_data->line_word.data));


	if (label_found) {
		switch (label_found->label_type){
		case LABEL_TYPE_COMMAND:
			printf("Command\n");
			break;
		case LABEL_TYPE_DATA:
			/*printf("%ld: ", utils_to_base4(line_data->decimal_address + LINE_OFFSET - 1 + IC));*/
			/*printf("Label: %s, Line: %d \n", label_found->label, label_found->line);*/
			break;
		case LABEL_TYPE_EXTERN:
			if (!extra_address_offset)
				fprintf(output_files[EXT_FILE], "%s\t%ld\n", label_found->label, to_base4(line_data->decimal_address + LINE_OFFSET - 1));
			break;
		}
	}
	fprintf(output_files[OB_FILE], "\n");
}

void _parser_translate_data(void* data) {
	LineData* line_data = data;
	_parser_translate_line(line_data, IC);
}

void _parser_translate_command(void* data) {
	LineData* line_data = data;
	_parser_translate_line(line_data, 0);
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

/* TEMP function. */
void _parser_print_data(void* data, FILE* stream) {
	LineData* line_data = data;

	printf("address: %d, label: %s\n", line_data->decimal_address, line_data->label_to_extract);
}

void parser_create_ent_file() {
	OpenFile(output_files[ENT_FILE], "ent");
	list_foreach(parser_entry_symbols, &_parser_find_label_address);
	fclose(output_files[ENT_FILE]);
}

void _parser_find_label_address(void* data) {
	unsigned long address;
	Label* label = data;
	Label* label_found;
	label_found = list_find_item(parser_symbols, label, &_parser_compare_labels);
	if (!label_found) {
		error_set("Error", "Entry label declared but not defined.", label->line);
		return;
	}
	address = label_found->line + LINE_OFFSET - 1;
	if (label_found->label_type == LABEL_TYPE_DATA)
		address += IC;

	fprintf(output_files[ENT_FILE], "%s\t%ld\n", label_found->label, to_base4(address));
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

void _parser_print_label(void* data, FILE* stream) {
	Label* label = data;
	fprintf(stream, "%d: %s\n", label->line, label->label);
}

void _parser_print_data_item(void* data, FILE* stream) {
	LineData* line_data = data;
	fprintf(stream, "decimal address: %d\nLabel to extract: %s\nData: %ld\n\n", line_data->decimal_address, line_data->label_to_extract, line_data->line_word.data);
}

void extract_data_number(char * begin_of_word, int const line_num) {
	int num_of_param, num_of_comma;
	long data_number;
	LineData* line_data;
	char * end_of_word;

	num_of_param = 0;
	num_of_comma = 0;

	while (*end_of_word != '\0') {
		NextWord(begin_of_word);
		end_of_word=begin_of_word;
		if (*begin_of_word == '\0' && num_of_param == 0)
			error_set("Warning", "Data line contains no data.", line_num);

		if (*begin_of_word == '\0')
			continue;
		
		if (*begin_of_word != '-' && *begin_of_word != '+' && !isdigit(*begin_of_word)) {
			error_set("Error", "Data line contain illegal number", line_num);
			continue;
		}

		end_of_word++;
		data_number = 0;
		if (isdigit(*begin_of_word))
			data_number = *begin_of_word - '0';

		while (!char_isblank(*end_of_word) && *end_of_word != '\0' && *end_of_word != ',') {
			if (!isdigit(*end_of_word)) {
				error_set("Error", "data line contain illegal number", line_num);
				continue;
			} 
			else {
				/* @TODO: Explain. */
				data_number = 10 * data_number + *end_of_word - '0';
				if ((*begin_of_word == '-' && data_number > -1 * MIN_DATA_NUMBER) || data_number > MAX_DATA_NUMBER) {
					error_set("Error", "number is out of limit", line_num);
					while (!char_isblank(*end_of_word) && *end_of_word != '\0' && *end_of_word != ',')
						end_of_word++;

					continue;
				}
			}
			end_of_word++;
		}

		if (*begin_of_word == '-')
			data_number *= -1;

		DC++;
		line_data = New(LineData);
		line_data->decimal_address = DC;
		line_data->line_word.data = data_number;
		data_list = list_append(data_list, line_data);

		num_of_param++;
		
		/*if (data_number >= MIN_DATA_NUMBER && data_number <= MAX_DATA_NUMBER)
			printf("\n data is %ld\n", data_number);*/

		NextWord(end_of_word);

		if (*end_of_word == ',') {
			end_of_word++;
			num_of_comma++;
		}
	}
	if (num_of_comma != num_of_param - 1) {
		error_set("Warning", "Data line contain spare comma at the end.", line_num);
	}
}

int extract_string(char* begin_of_word, int const line_num, char* line) {
	LineData* line_data;
	char * end_of_word;
	NextWord(begin_of_word);

	if (*begin_of_word != '"') {
		error_set("Error", "String expected after .string", line_num);
		return 0;
	}

	end_of_word = line + strlen(line) - 1;

	while (char_isblank(*end_of_word))
		end_of_word--;

	if (*(end_of_word) != '"' || end_of_word == begin_of_word) {
		error_set("Error", "String expected after .string", line_num);
		return 0;
	}

	while (begin_of_word + 1 < end_of_word) {
		DC++;
		line_data = New(LineData);
		line_data->decimal_address = IC;
		line_data->line_word.data = *(begin_of_word + 1);
		data_list = list_append(data_list, line_data);

		begin_of_word++;
	}

	DC++;			
	line_data = New(LineData);
	line_data->decimal_address = IC;
	line_data->line_word.data = 0;
	data_list = list_append(data_list, line_data);

	return 0;
}

void extract_label(char * begin_of_word, char *end_of_word, int const line_num, char * line, LineType line_type) {
	Label* label;
	begin_of_word = end_of_word;

	NextWord(begin_of_word);

	if (!isalpha(*begin_of_word)) {
		error_set("Error", "Not a legal label", line_num);
		return;
	}

	end_of_word = line + strlen(line) - 1;
	while (char_isblank(*end_of_word))
		end_of_word--;

	/* Check if label name is a register name. */
	if ((end_of_word - begin_of_word == 1 && ((*begin_of_word == 'r' && (*end_of_word - '0') >= 0 && (*end_of_word - '0') <= 7) || !strncmp(begin_of_word, "PC", 2) || !strncmp(begin_of_word, "SP", 2))) || (end_of_word - begin_of_word == 2 && !strncmp(begin_of_word, "PSW", 3))) {
		error_set("Error", "Illegal label name, same as reg.", line_num);
		return;
	}

	while (isalnum(*end_of_word))
		end_of_word--;

	if (end_of_word > begin_of_word) {
		error_set("Error", "Label expected", line_num);
		return;
	}

	/* Insert the label to the external labels list or to the entry labels list.
	 */
	label = New(Label);

	label->label = (char*)malloc(MAX_LABEL_SIZE + 1);
	if (!label->label)
		error_fatal(ErrorMemoryAlloc);
	strcpy(label->label, begin_of_word);

	switch (line_type) {
	case LINE_TYPE_ENTRY:
		label->line = line_num;
		parser_entry_symbols = list_add_ordered(parser_entry_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		break;

	case LINE_TYPE_EXTERN:
		label->label_type = LABEL_TYPE_EXTERN;
		label->line = 0;
		parser_symbols = list_add_ordered(parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		break;
	}
}

long extract_number(char number[MAX_LABEL_SIZE + 1], const int line_num) {
	long data_number;
	int k = 0;
	
	if (number[0] != '-' && number[0] != '+' && !isdigit(number[0])) {
		error_set("Error", "Illegal operand, expect number after.", line_num);
		return 999999;
	}
	
	data_number = 0;
	if (isdigit(number[k]))
		data_number = number[k] - '0';
	k++;

	while (number[k] != '\0') {
		if (!isdigit(number[k])) {
			error_set("Error", "Illegal number.", line_num);
			return 999999;
		} 
		
		else {
			data_number = 10 * data_number + number[k] - '0';
			if ((number[0] == '-' && data_number > -1 * MIN_DATA_NUMBER) || data_number > MAX_DATA_NUMBER) {
				error_set("Error", "Number out of limit.", line_num);
				return 999999;
			}
		}
		k++;
	}

	if (number[0] == '-')
		return data_number * (-1);

	return data_number;
}

int extract_operand(char *operand,int i,int line_num ) {
	int j;

	if (*end_of_word != '#' && !isalpha(*end_of_word)) {
		error_set("Error", "Illegal parameter.", line_num);
		return 0;
	}

	*operand = *end_of_word;
	end_of_word++;
	j = 1;
	while (isalnum(*end_of_word) && j <= MAX_LABEL_SIZE) {	
		operand[j] = *end_of_word;
		end_of_word++;
		j++;
	}

	if (j == MAX_LABEL_SIZE + 1) {
		error_set("Error", "Illegal operand.", line_num);
		return 0;
	}

	operand[j] = '\0';

	/* Assuming no white chars at middle of operand. */
	if ((*end_of_word != '{' && *end_of_word != ',' && *end_of_word != '\0' && !char_isblank(*end_of_word)) || (operand[0] == '#' && *end_of_word == '{')) {
		error_set("Error", "Illegal parameter.", line_num);
		return 0;
	}
	return 1;
}

int extract_operand_offset(char * operand_offset,int i,int line_num) {
	int j;

	/* Extracting offset for first parameter if any. */
	end_of_word++;

	j = 0;
	while (isalnum(*end_of_word) && j <= MAX_LABEL_SIZE) {	
		operand_offset[j] = *end_of_word;
		end_of_word++;
		j++;
	}

	if (j == MAX_LABEL_SIZE + 1 || *end_of_word != '}') {
		error_set("Error", "Illegal operand.", line_num);
		return 0;
	}

	operand_offset[j] = '\0';
	end_of_word++;
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

			IC++;
			line_data = New(LineData);
			line_data->decimal_address = IC;
			line_data->label_to_extract = NULL;
			commands_list = list_append(commands_list, line_data);

			if((line_data->line_word.data = extract_number(&operand[1], line_num)) == 999999)
				return 0;
			break;

		case 1:
			if ((!commands[i].src_direct_address && work_on_src) || (!commands[i].dest_direct_address && !work_on_src)) {
				error_set("Error", "Illegal address.", line_num);
				return 0;
			}

			IC++;
			line_data = New(LineData);
			line_data->decimal_address = IC;
			line_data->line_word.data = 0;
			commands_list = list_append(commands_list, line_data);

			line_data->label_to_extract = (char*)malloc(strlen(operand) + 1);
			strcpy(line_data->label_to_extract, operand);
			break;

		case 2:
			if ((!commands[i].src_index_address && work_on_src) || (!commands[i].dest_index_address && !work_on_src)) {
				error_set("Error", "Illegal address.", line_num);
				return 0;
			}

			IC++;
			line_data = New(LineData);
			line_data->decimal_address = IC;
			line_data->line_word.data = 0;
			commands_list = list_append(commands_list, line_data);
			
			line_data->label_to_extract = (char*)malloc(strlen(operand) + 1);
			strcpy(line_data->label_to_extract, operand);

			if (isdigit(*operand_offset)) {
				IC++;
				line_data = New(LineData);
				line_data->decimal_address = IC;
				line_data->label_to_extract = NULL;
				commands_list = list_append(commands_list, line_data);
			
				if((line_data->line_word.data = extract_number(operand_offset, line_num)) == 999999) {
					error_set("Error", "Illegal address.", line_num);
					return 0;
				}
			}
			else if (!(*operand_offset == 'r' && strlen(operand_offset) == 2 && *(operand_offset + 1) >= '0' && *(operand_offset + 1) <= '7')) {
				IC++;
				line_data = New(LineData);
				line_data->decimal_address = IC;
				line_data->line_word.data = 0;
				commands_list = list_append(commands_list, line_data);
				
				line_data->label_to_extract = (char*)malloc(strlen(operand_offset) + 1);
				strcpy(line_data->label_to_extract, operand_offset);
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
