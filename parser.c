#include "char.h"
#include "error.h"
#include "list.h"
#include "parser.h"
#include "reader.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List parser_symbols_list;
List parser_extern_symbols;
List parser_entry_symbols;
List lines_data_list;

int IC = LINE_OFSET; /* Instruction counter */

void parser_parse() {
	const Instruction instruction_list[] = {
		{0, 2, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, "mov"},
		{1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, "cmp"},
		{2, 2, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, "add"},
		{3, 2, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, "sub"},
		{6, 2, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, "lea"},
		{4, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "not"},
		{5, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "clr"},
		{7, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "inc"},
		{8, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "dec"},
		{9, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "jmp"},
		{10, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "bne"},
		{11, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "red"},
		{12, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, "prn"},
		{13, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, "jsr"},
		{14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "rts"},
		{15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "stop"}
	};

	char* line, first_opperand[MAX_LABEL_SIZE + 1];
	Label* label;
	line_parse* line_data;
	char *begin_of_word, *end_of_word, command_type[7];
	int line_num = 0, i, j;

	long data_number;
	void extract_data_number(char *, int);
	int extract_string(char *, int, char *);
	int extract_label(char*, char *, int, char *, LineType);
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

		if (label->label) { /*find begining of next word after label */
			begin_of_word = line + strlen(label->label) + 1;
			find_next_non_blank_char(begin_of_word);
		}

		if (*begin_of_word == '\0' && label->label) {
			/*assuming every label declaration must folow instructions or declation of data */
			error_set("Error", "label with no instrucion", line_num);
			free(label);
			continue;
		}

		/* find end of command */
		end_of_word = begin_of_word + 1;
		while (!char_isblank(*end_of_word) && *end_of_word != '/' && *end_of_word != '\0')
			end_of_word++;

		if (((!strncmp(begin_of_word, ".data", 5) && (end_of_word - begin_of_word) == 5 && *end_of_word != '/')
				|| (!strncmp(begin_of_word, ".string", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/'))
				&& label->label) {
			label->is_data = 1;
			parser_symbols_list = list_add_ordered(parser_symbols_list, label, &_parser_compare_labels, &_parser_duplicated_label);

		}

		/* this is a data line */
		if (!strncmp(begin_of_word, ".data", 5) && (end_of_word - begin_of_word) == 5 && *end_of_word != '/') {
			extract_data_number(end_of_word + 1, line_num);
			continue;
		}

		/* this is a string line */

		if (!strncmp(begin_of_word, ".string", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/') {
			extract_string(end_of_word + 1, line_num, line);
			continue;
		}


		/* this is an entry label declaration line */
		if (!strncmp(begin_of_word, ".entry", 6) && (end_of_word - begin_of_word) == 6 && *end_of_word != '/') {
			extract_label(begin_of_word, end_of_word, line_num, line, LINE_TYPE_ENTRY);
			continue;
		}


		/* this is an extern label declaration line */

		if (!strncmp(begin_of_word, ".extern", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/') {
			extract_label(begin_of_word, end_of_word, line_num, line, LINE_TYPE_EXTERN);
			continue;
		}

		line_data = New(line_parse);
		line_data->decimal_address = IC;
		line_data->line_word.data.data = 0;
		line_data->label_to_extract = NULL;
		lines_data_list = list_append(lines_data_list, line_data);
		IC++;

		/* this is a command line*/
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

		/* verify blank after type */
		if (!char_isblank(*end_of_word) && *end_of_word != '\0') {
			error_set("Error", "Blank char is required after instruction.", line_num);
			continue;
		}

		/* find first opperand */

		find_next_non_blank_char(end_of_word);


		if (*end_of_word == '\0' && (instruction_list[i].source_opperand ||
				instruction_list[i].destination_opperand)) {
			printf("error at line number %d opperand expected after %s", line_num,
					instruction_list[i].instruction);
			continue;
		}

		if (*end_of_word != '\0' && instruction_list[i].source_opperand == 0 &&
				instruction_list[i].destination_opperand == 0) {

			printf("error at line number %d no opperand expected after %s", line_num,
					instruction_list[i].instruction);
			continue;
		}

		if (*end_of_word == '\0' && instruction_list[i].source_opperand == 0 &&
				instruction_list[i].destination_opperand == 0) {
			/* a command without oppenrand found */
			printf("\n%d label is %s command is %s line is %s\n", line_num, label->label, instruction_list[i].instruction, line);
			continue;
		}

		if (*end_of_word != '#' && !isalpha(*end_of_word)) {

			printf("\nilegal parameter at line%d\n", line_num);
			continue;
		}

		j = 1;
		while (isalnum(*end_of_word) && j <= MAX_OPERAND_SIZE) {
			first_opperand[j - 1] = *end_of_word;
			end_of_word++;
			j++;
		}

		if (j == MAX_OPERAND_SIZE) {
			printf("error at line number %d ilegal opperand after %s", line_num,
					instruction_list[i].instruction);
			continue;
		}

		first_opperand[j - 1] = '\0';
		find_next_non_blank_char(end_of_word);
		if (*end_of_word != '{' && *end_of_word != ',' && *end_of_word != '\0') {

			printf("\nilegal parameter at line%d\n", line_num);
			continue;
		}

		if (*end_of_word == '\0') {
			/*there is only one parameter*/
			if (!(instruction_list[i].source_opperand ^ instruction_list[i].destination_opperand)) {
				printf("\nincompatble number of parameters %d\n", line_num);
				continue;
			}

			if (*end_of_word == '#')
				/*addressing is value 0*/
				data_number = extract_number(first_opperand, line_num);
			/* else this is a label or register*/

			continue;
		}


		find_next_non_blank_char(end_of_word);
		if (*end_of_word != '\0' && *end_of_word != ',' && *end_of_word != '{') {
			printf("error at line number %d ilegal opperand after %s", line_num,
					instruction_list[i].instruction);
			continue;
		}

		if (*end_of_word != '{') {

			while (*end_of_word != ',' && !char_isblank(*end_of_word) && j <= MAX_OPERAND_SIZE) {
				first_opperand[j - 1] = *end_of_word;
				end_of_word++;
				j++;
			}
		}

		printf("\n%d label is %s command is %s first parameter is %ld line is %s\n", line_num, label->label, instruction_list[i].instruction, data_number, line);
		free(line);
	}

	/*error_print_list();*/
	list_print(parser_symbols_list, &_parser_print_label);
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

int _parser_compare_labels(void* a, void* b) {
	Label* label = a;
	Label* label2 = b;
	return strcmp(label->label, label2->label);
}

void _parser_duplicated_label(void* data) {
	Label* label = data;
	error_set("Error", "Redeclaring label.", label->line);
}

void _parser_print_label(void* data) {
	Label* label = data;
	printf("%d: %s\n", label->line, label->label);
}

void extract_data_number(char * begin_of_word, int const line_num) {
	int num_of_param, num_of_comma;
	long data_number;
	line_parse* line_data;
	char * end_of_word;

	num_of_param = 0;
	num_of_comma = 0;
	printf("\nthis is a data line\n");

	while (*end_of_word != '\0') {
		find_next_non_blank_char(begin_of_word);
		end_of_word = begin_of_word;
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

		line_data = New(line_parse);
		line_data->decimal_address = IC;
		line_data->line_word.data.data = data_number;
		lines_data_list = list_append(lines_data_list, line_data);
		IC++;
		num_of_param++;

		if (data_number >= MIN_DATA_NUMBER && data_number <= MAX_DATA_NUMBER)
			printf("\n data is %ld\n", data_number);

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

	printf("\nthis is a string line, the string is %s\n", begin_of_word);
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

int extract_label(char * begin_of_word, char *end_of_word, int const line_num, char * line, LineType line_type) {
	begin_of_word = end_of_word;

	find_next_non_blank_char(begin_of_word);

	if (!isalpha(*begin_of_word)) {
		error_set("Error", "Illegal label.", line_num);
		return 0;
	}

	end_of_word = line + strlen(line) - 1;
	while (char_isblank(*end_of_word))
		end_of_word--;


	/* check if label name is a register name*/
	if ((end_of_word - begin_of_word == 1 && ((*begin_of_word == 'r' && (*end_of_word - '0') >= 0 && (*end_of_word - '0') <= 7) || !strncmp(begin_of_word, "PC", 2) || !strncmp(begin_of_word, "SP", 2))) || (end_of_word - begin_of_word == 2 && !strncmp(begin_of_word, "PSW", 3))) {
		error_set("Error", "Illegal label name, same as register.", line_num);
		return 0;
	}

	while (isalnum(*end_of_word))
		end_of_word--;

	if (end_of_word > begin_of_word) {
		error_set("Error", "Label expected.", line_num);
		return 0;
	}


	printf("\nthe label is %s\n", begin_of_word);

	return 0;
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
