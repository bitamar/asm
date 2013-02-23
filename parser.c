#include "char.h"
#include "error.h"
#include "list.h"
#include "parser.h"
#include "reader.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List parser_symbols;
List parser_extern_symbols;
List parser_entry_symbols;
List lines_data_list;

int IC = LINE_OFSET; /* Instruction counter */

void parser_parse() {
	const Instruction instruction_list[] = {
		{2, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, "mov"},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, "cmp"},
		{2, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, "add"},
		{2, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, "sub"},
		{2, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, "lea"},
		{1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "not"},
		{1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "clr"},
		{1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "inc"},
		{1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "dec"},
		{1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "jmp"},
		{1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "bne"},
		{1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "red"},
		{1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, "prn"},
		{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, "jsr"},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "rts"},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "stop"}
	};

	char* line, first_operand[MAX_LABEL_SIZE + 1];
	Label* label;
	line_parse* line_data;
	char *begin_of_word, *end_of_word, command_type[7];
	int line_num = 0, i, j;

	long data_number;
	void extract_data_number(char *, int);
	int extract_string(char *, int, char *);
	void extract_label(char*, char *, int, char *, LineType);
	long extract_number(char *, int const);

	while ((line = reader_get_line())) {
		line_num++;

		/* this is for remark line */
		if (*line == ';')
			continue;

		label = New(Label);
		label->label = parser_get_label(line, line_num);
		label->line = IC;
		label->is_data = 0;
		begin_of_word = line;

		find_next_non_blank_char(begin_of_word);

		/* this is for empty line */
		if (*begin_of_word == '\0') {
			free(label);
			continue;
		}

		/* Find beginning of next word after label. */
		if (label->label) {
			begin_of_word = line + strlen(label->label) + 1;
			find_next_non_blank_char(begin_of_word);
		}

		if (*begin_of_word == '\0' && label->label) {
			/* Assuming every label declaration must follow an instructions or declaration of data. */
			error_set("Error", "Label with no instruction.", line_num);
			free(label);
			continue;
		}

		/* Find end of command. */
		end_of_word = begin_of_word + 1;
		while (!char_isblank(*end_of_word) && *end_of_word != '/' && *end_of_word != '\0')
			end_of_word++;

		if (((!strncmp(begin_of_word, ".data", 5) && (end_of_word - begin_of_word) == 5 && *end_of_word != '/')
				|| (!strncmp(begin_of_word, ".string", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/'))
				&& label->label) {
			label->is_data = 1;
			parser_symbols = list_add_ordered(parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);

		}
		
		line_data = New(line_parse);
		line_data->decimal_address = IC++;
		line_data->line_word.data.data = 0;
		line_data->label_to_extract = NULL;
		lines_data_list = list_append(lines_data_list, line_data);

		/* This is a data line. */
		if (!strncmp(begin_of_word, ".data", 5) && (end_of_word - begin_of_word) == 5 && *end_of_word != '/') {
			extract_data_number(end_of_word + 1, line_num);
			continue;
		}

		/* This is a string line. */
		if (!strncmp(begin_of_word, ".string", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/') {
			extract_string(end_of_word + 1, line_num, line);
			continue;
		}

		/* This is an entry label declaration line. */
		if (!strncmp(begin_of_word, ".entry", 6) && (end_of_word - begin_of_word) == 6 && *end_of_word != '/') {
			extract_label(begin_of_word, end_of_word, line_num, line, LINE_TYPE_ENTRY);
			continue;
		}

		/* This is an external label declaration line. */
		if (!strncmp(begin_of_word, ".extern", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/') {
			extract_label(begin_of_word, end_of_word, line_num, line, LINE_TYPE_EXTERN);
			continue;
		}

		/* This is a command line. */
		if (strlen(line) > 80) {
			error_set("Error", "line length exceeding 80 char", line_num);
			continue;
		}

		for (i = 0; i < 16 && strncmp(begin_of_word, instruction_list[i].instruction, end_of_word - begin_of_word); i++);
		if (i == 16) {
			error_set("Error", "Unknown instruction", line_num);
			continue;
		}

		/* find '/0' or /1 */
		find_next_non_blank_char(end_of_word);
		command_type[0] = *end_of_word;
		if (*end_of_word != '\0') {
			end_of_word++;
			find_next_non_blank_char(end_of_word);
			command_type[1] = *end_of_word;
			command_type[2] = '\0';
		}
		if (!(command_type[0] == '/' && (command_type[1] == '0' || command_type[1] == '1'))) {
			error_set("Error", "Type expected after instruction.", line_num);
			continue;
		}

		if (*end_of_word != '\0')
			end_of_word++;

		if (command_type[1] == '1') {
			for (j = 0; j < 4; j++) {
				find_next_non_blank_char(end_of_word);
				command_type[2 + j] = *end_of_word;
				command_type[2 + j + 1] = '\0';
				if (*end_of_word != '\0')
					end_of_word++;
			}

			if (!(command_type[0] == '/' && command_type[2] == '/' && command_type[4] == '/' &&
					(command_type[5] == '0' || command_type[5] == '1') &&
					(command_type[3] == '0' || command_type[3] == '1'))) {
				error_set("Error", "Illegal command type", line_num);
				continue;
			}
		}

		/* Verify blank after type. */
		if (!char_isblank(*end_of_word) && *end_of_word != '\0') {
			error_set("Error", "Blank char is required after instruction.", line_num);
			continue;
		}

		/* Find the first operand. */
		find_next_non_blank_char(end_of_word);

		if (*end_of_word == '\0' && (instruction_list[i].source_operand ||
				instruction_list[i].destination_operand)) {
			printf("error at line number %d operand expected after %s", line_num, instruction_list[i].instruction);
			continue;
		}

		if (*end_of_word != '\0' && instruction_list[i].source_operand == 0 && instruction_list[i].destination_operand == 0) {
			printf("error at line number %d no operand expected after %s", line_num, instruction_list[i].instruction);
			continue;
		}

		if (*end_of_word == '\0' && instruction_list[i].source_operand == 0 &&	instruction_list[i].destination_operand == 0) {
			/* Command without operand found. */
			printf("\n%d label is %s command is %s line is %s\n", line_num, label->label, instruction_list[i].instruction, line);
			continue;
		}

		if (*end_of_word != '#' && !isalpha(*end_of_word)) {
			printf("\nIllegal parameter at line%d.\n", line_num);
			continue;
		}

		j = 1;
		while (isalnum(*end_of_word) && j <= MAX_OPERAND_SIZE) {
			first_operand[j - 1] = *end_of_word;
			end_of_word++;
			j++;
		}

		if (j == MAX_OPERAND_SIZE) {
			printf("Error at line number %d illegal operand after %s.", line_num, instruction_list[i].instruction);
			continue;
		}

		first_operand[j - 1] = '\0';
		find_next_non_blank_char(end_of_word);
		if (*end_of_word != '{' && *end_of_word != ',' && *end_of_word != '\0') {
			printf("\nIllegal parameter at line%d\n", line_num);
			continue;
		}

		if (*end_of_word == '\0') {
			/* There is only one parameter. */
			if (!(instruction_list[i].source_operand ^ instruction_list[i].destination_operand)) {
				printf("\nIncompatble number of parameters %d\n", line_num);
				continue;
			}

			if (*end_of_word == '#')
				/* Addressing is value 0. */
				data_number = extract_number(first_operand, line_num);
				/* Otherwise this is a label or register. */
			continue;
		}


		find_next_non_blank_char(end_of_word);
		if (*end_of_word != '\0' && *end_of_word != ',' && *end_of_word != '{') {
			printf("error at line number %d illegal operand after %s", line_num,
					instruction_list[i].instruction);
			continue;
		}

		if (*end_of_word != '{') {

			while (*end_of_word != ',' && !char_isblank(*end_of_word) && j <= MAX_OPERAND_SIZE) {
				first_operand[j - 1] = *end_of_word;
				end_of_word++;
				j++;
			}
		}

		printf("\n%d label is %s command is %s first parameter is %ld line is %s\n", line_num, label->label, instruction_list[i].instruction, data_number, line);
		free(line);
	}

	/*list_print(parser_symbols, &_parser_print_label);*/
	/*list_print(parser_entry_symbols, &_parser_print_label);*/
}

char* parser_get_label(const char* line, int line_num) {
	int len = 0;
	char* label;
	const char* c = line;

	label = (char*) malloc(MAX_LABEL_SIZE + 1);
	if (!label)
		error_fatal(ErrorMemoryAlloc);

	/* Iterate the first word in the line, until colon, space or end of line is
	 * found. */
	if (!isalpha(*c)) {
		free(label);
		return NULL;
	}

	while (isalnum(*c) && len < MAX_LABEL_SIZE) {
		c++;
		len++;
	}

	if (line[len] == ':') {
		strncpy(label, line, len);
		label[len] = '\0';

		/* check if label name is a register name*/
		if ((strlen(label) == 2 && label[0] == 'r' && (label[1] - '0') >= 0 && (label[1] - '0') <= 7) || !strcmp(label, "PC") || !strcmp(label, "SP") || !strcmp(label, "PSW")) {
			error_set("Error", "Illegal label name, same as register.", line_num);
			return NULL;
		}

		return label;
	}

	free(label);
	return NULL;
}

void perser_output_ext_file() {
	FILE* ext_file = fopen("file.ext", "w");
	if (!ext_file) {
		fprintf(stderr, ErrorCantRead, "file.ext");
		exit(EXIT_FAILURE);
	}
	list_print(parser_extern_symbols, ext_file, &_parser_print_label);
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

void extract_data_number(char * begin_of_word, int const line_num) {
	int num_of_param, num_of_comma;
	long data_number;
	line_parse* line_data;
	char * end_of_word;

	num_of_param = 0;
	num_of_comma = 0;
	printf("\nData line\n");

	while (*end_of_word != '\0') {
		find_next_non_blank_char(begin_of_word);
		end_of_word = begin_of_word;
		if (*begin_of_word == '\0' && num_of_param == 0)
			error_set("Warning", "Data line contains no data.", line_num);

		if (*begin_of_word == '\0')
			continue;

		if (*begin_of_word != '-' && *begin_of_word != '+' && !isdigit(*begin_of_word)) {
			error_set("Error", "Data line contains an illegal number.", line_num);
			continue;
		}

		end_of_word++;
		data_number = 0;
		if (isdigit(*begin_of_word))
			data_number = *begin_of_word - '0';

		while (!char_isblank(*end_of_word) && *end_of_word != '\0' && *end_of_word != ',') {
			if (!isdigit(*end_of_word)) {
				error_set("Error", "Data line contains an illegal number.", line_num);
				continue;
			}
			else {
				/* @TODO: Explain. */
				data_number = 10 * data_number + *end_of_word - '0';
				if ((*begin_of_word == '-' && data_number > -1 * MIN_DATA_NUMBER) || data_number > MAX_DATA_NUMBER) {
					error_set("Error", "Number is out of limit.", line_num);
					while (!char_isblank(*end_of_word) && *end_of_word != '\0' && *end_of_word != ',')
						end_of_word++;

					continue;
				}
			}
			end_of_word++;
		}

		if (*begin_of_word == '-')
			data_number *= -1;

		line_data = New(line_parse);
		line_data->decimal_address = IC;
		line_data->line_word.data.data = data_number;
		lines_data_list = list_append(lines_data_list, line_data);
		IC++;
		num_of_param++;

		if (data_number >= MIN_DATA_NUMBER && data_number <= MAX_DATA_NUMBER)
			printf("\nData is %ld\n", data_number);

		find_next_non_blank_char(end_of_word);


		if (*end_of_word == ',') {
			end_of_word++;
			num_of_comma++;
		}
	}
	if (num_of_comma != num_of_param - 1) {
		error_set("Warning", "Data line contain spare comma at the end.", line_num);
	}
}

int extract_string(char * begin_of_word, int const line_num, char * line) {
	line_parse* line_data;
	char * end_of_word;
	find_next_non_blank_char(begin_of_word);

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

	printf("\nString line: The string is %s.\n", begin_of_word);
	while (begin_of_word + 1 < end_of_word) {
		line_data = New(line_parse);
		line_data->decimal_address = IC;
		line_data->line_word.data.data = *(begin_of_word + 1);
		lines_data_list = list_append(lines_data_list, line_data);

		begin_of_word++;
		IC++;
	}
	line_data = New(line_parse);
	line_data->decimal_address = IC;
	line_data->line_word.data.data = 0;
	lines_data_list = list_append(lines_data_list, line_data);

	begin_of_word++;
	IC++;
	return 0;
}

void extract_label(char* begin_of_word, char* end_of_word, const int line_num, char * line, LineType line_type) {
	Label* label;

	begin_of_word = end_of_word;

	find_next_non_blank_char(begin_of_word);

	if (!isalpha(*begin_of_word)) {
		error_set("Error", "Illegal label.", line_num);
		return;
	}

	end_of_word = line + strlen(line) - 1;
	while (char_isblank(*end_of_word))
		end_of_word--;


	/* check if label name is a register name*/
	if ((end_of_word - begin_of_word == 1 && ((*begin_of_word == 'r' && (*end_of_word - '0') >= 0 && (*end_of_word - '0') <= 7) || !strncmp(begin_of_word, "PC", 2) || !strncmp(begin_of_word, "SP", 2))) || (end_of_word - begin_of_word == 2 && !strncmp(begin_of_word, "PSW", 3))) {
		error_set("Error", "Illegal label name, same as register.", line_num);
		return;
	}

	while (isalnum(*end_of_word))
		end_of_word--;

	if (end_of_word > begin_of_word) {
		error_set("Error", "Label expected.", line_num);
		return;
	}

	/* Insert the label to the external labels list or to the entry labels list.
	 */
	label = New(Label);
	label->line = line_num;
	label->label = (char*)malloc(MAX_LABEL_SIZE + 1);
	if (!label->label)
		error_fatal(ErrorMemoryAlloc);
	strcpy(label->label, begin_of_word);
	switch (line_type) {
	case LINE_TYPE_ENTRY:
		parser_entry_symbols = list_append(parser_entry_symbols, label);
		break;
	case LINE_TYPE_EXTERN:
		parser_extern_symbols = list_append(parser_extern_symbols, label);
		break;
	}
}

long extract_number(char number[MAX_LABEL_SIZE + 1], const int line_num) {
	long data_number;
	int k = 1;

	if (number[k] != '-' && number[k] != '+' && !isdigit(number[k])) {
		error_set("Error", "Illegal operand, expect number after.", line_num);
		return 999999;
	}

	k++;
	data_number = 0;
	if (isdigit(number[k]))
		data_number = number[k] - '0';

	while (number[k] != '\0') {
		if (!isdigit(number[k])) {
			error_set("Error", "Illegal number after #.", line_num);
			return 999999;
		} else {
			data_number = 10 * data_number + number[k] - '0';
			if ((*number == '-' && data_number > -1 * MIN_DATA_NUMBER) || data_number > MAX_DATA_NUMBER) {
				error_set("Error", "Number out of limit.", line_num);
				return 999999;
			}
		}
		k++;
	}

	if (*number == '-')
		return data_number * (-1);

	return data_number;
}
