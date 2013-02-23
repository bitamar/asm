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
line_parse* line_data;
int IC = LINE_OFSET; /* Instruction counter */

const Instruction instruction_list[] = {
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1, "mov"},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, "cmp"},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1, "add"},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1, "sub"},
	{0, 1, 1, 1, 0, 1, 1, 1, 1, 1, "lea"},
	{0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "not"},
	{0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "clr"},
	{0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "inc"},
	{0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "dec"},
	{0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "jmp"},
	{0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "bne"},
	{0, 0, 0, 0, 0, 1, 1, 1, 0, 1, "red"},
	{0, 0, 0, 0, 1, 1, 1, 1, 0, 1, "prn"},
	{0, 0, 0, 0, 0, 1, 0, 0, 0, 1, "jsr"},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "rts"},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "stop"}
};
char * end_of_word;
void parser_parse() {

	char* line, operand1[MAX_LABEL_SIZE + 1],operand1_ofset[MAX_LABEL_SIZE + 1],
		operand2[MAX_LABEL_SIZE + 1],operand2_ofset[MAX_LABEL_SIZE + 1];
	Label* label;
	
	char *begin_of_word, command_type[7];
	int line_num = 0, i, j;
	void extract_data_number(char *, int);
	int extract_string(char *, int, char *);
	void extract_label(char * begin_of_word, char *end_of_word, int const line_num, char * line, LineType line_type);
	long extract_number(char *, int const);
	int extract_operand(char *,int i,int line_num );
	int extract_operand_ofset(char * ,int i,int line_num);
	int update_operand(char *,char *,int);
	int add_operand_lines (char *,char *,int,int,int);

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
			free (label);
			continue;
		}

		if (label->label) { /*find begining of next word after label */		
			begin_of_word = line + strlen(label->label) + 1;
			find_next_non_blank_char(begin_of_word);
		}

		if (*begin_of_word == '\0' && label->label) {
			/*assuming every label declaration must folow instructions or declation of data */
			error_set("Error", "label with no instrucion", line_num); 
			free (label);
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

			parser_symbols = list_add_ordered(parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);


		}			

		/* this is a data line */
		if (!strncmp(begin_of_word, ".data", 5) && (end_of_word - begin_of_word) == 5 && *end_of_word != '/') {
			
			extract_data_number(end_of_word, line_num);
			continue;
		}

		/* this is a string line */

		if (!strncmp(begin_of_word, ".string", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/') {
			extract_string(end_of_word, line_num, line);
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

		label->is_data = 0;

		if(label->label) 
			parser_symbols = list_add_ordered(parser_symbols, label, &_parser_compare_labels, &_parser_duplicated_label);
		
		
		if (strlen(line) > 80) {
			error_set("Error", "line length exceeding 80 char", line_num);

			continue;
		}

		for (i = 0; i < 16 && strncmp(begin_of_word, instruction_list[i].instruction, end_of_word - begin_of_word); i++);
		if (i == 16) {
			error_set("Error", "Unknown instruction", line_num);

			continue;
		}

		line_data->line_word.inst.opcode = i;
		/* find '/0' or /1 , asuming /0 is not folowed by other options */
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
			
			line_data->line_word.inst.type=command_type[1]-'0';
			line_data->line_word.inst.comb=command_type[5]-'0'+2*(command_type[3]-'0');
		}

		/* verify blank after type */
		if (!char_isblank(*end_of_word) && *end_of_word != '\0') {
			error_set("Error", "Blank char is required after instruction.", line_num);

			continue;
		}

		/* find operands */
		operand1[0]='\0';
		operand2[0]='\0';
		operand1_ofset[0]='\0';
		operand2_ofset[0]='\0';
		find_next_non_blank_char(end_of_word);

		if (*end_of_word!='\0') {
			if (!extract_operand(operand1,i,line_num))
				continue;
			if (*end_of_word=='{')
				if (!extract_operand_ofset(operand1_ofset,i,line_num))
					continue;
		}

		find_next_non_blank_char(end_of_word);
		
		if (*end_of_word==',') {
			
			end_of_word++;
			find_next_non_blank_char(end_of_word);
			if (!extract_operand(operand2,i,line_num))
				continue;
			if (*end_of_word=='{')
				if (!extract_operand_ofset(operand2_ofset,i,line_num))
					continue;
		}

		find_next_non_blank_char(end_of_word);

		if (*end_of_word!='\0'
		    || (*operand1=='#' && *operand1_ofset!='\0') || (*operand2=='#' && *operand2_ofset!='\0')) {
			printf("error at line number %d ilegal parameters", line_num);		
			continue;
		}

		/* chech when no operand required */
		if (!instruction_list[i].source_operand && !instruction_list[i].destination_operand && *operand1!='\0') {
			printf("error at line number %d no operands required", line_num);
			continue;
		}

		/* chech when one operand required */
		if (!instruction_list[i].source_operand && instruction_list[i].destination_operand &&
		    (*operand1=='\0' || *operand2!='\0' )) {
			printf("\nerror at line number %d ilegal number of operands, required 1\n", line_num);
			continue;
		}

		/* chech when two operand required */
		if (instruction_list[i].source_operand && instruction_list[i].destination_operand &&
		    (*operand1=='\0' || *operand2=='\0' )) {
			printf("\nerror at line number %d ilegal number of operands, required 2\n", line_num);
			continue;
		}

		/* addressing dealing */

		/* when two operand exists*/
		if (*operand2!='\0') {
			update_operand(operand1,operand1_ofset,1);
			update_operand(operand1,operand1_ofset,0);
			if (!add_operand_lines (operand1,operand1_ofset,1,i,line_num))
				continue;
			if (!add_operand_lines (operand2,operand2_ofset,0,i,line_num))
				continue;
		}

		/* when one operand exists*/
		if (*operand1!='\0' && *operand2=='\0') {
			update_operand(operand1,operand1_ofset,0);
			if (!add_operand_lines (operand1,operand1_ofset,0,i,line_num))
				continue;
		}
		
		printf("\n%d label is %s first parameter is %s,%s ,second %s,%s line is %s\n", line_num, label->label, operand1,operand1_ofset,operand2,operand2_ofset, line);
	}

	list_print(parser_symbols, stdout, &_parser_print_label);
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
	printf("\nthis is a data line\n");

	while (*end_of_word != '\0') {
		find_next_non_blank_char(begin_of_word);
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
	IC++;
		
	return 0;
}

void extract_label(char * begin_of_word, char *end_of_word, int const line_num, char * line, LineType line_type) {
	Label* label;
	begin_of_word = end_of_word;

	find_next_non_blank_char(begin_of_word);

	if (!isalpha(*begin_of_word)) {
		error_set("Error", "Not a legal label", line_num);
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
		error_set("Error", "Label expected", line_num);
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
			error_set("Error", "Illegal number after #.", line_num);
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

		printf("\nilegal parameter at line %d\n", line_num);
		return 0;
	}

	*operand=*end_of_word;
	end_of_word++;
	j = 1;
	while (isalnum(*end_of_word) && j <= MAX_LABEL_SIZE) {	
		operand[j] = *end_of_word;
		end_of_word++;
		j++;
	}

	if (j == MAX_LABEL_SIZE+1) {
		printf("error at line number %d ilegal operand after %s", line_num,
				instruction_list[i].instruction);
		return 0;
		}

	operand[j] = '\0';

	/* assumind no white chars at middle of opperan*/
	if ((*end_of_word != '{' && *end_of_word != ',' && *end_of_word != '\0' && !char_isblank(*end_of_word)) || 
	    (operand[0]=='#' && *end_of_word == '{')) {

		printf("\nilegal parameter at line%d\n",line_num);
		return 0;
	}
	return 1;
}
int extract_operand_ofset(char * operand_ofset,int i,int line_num) {
	int j;
	/* extracting ofset for first parameter if any */

	end_of_word++;

	j = 0;
	while (isalnum(*end_of_word) && j <= MAX_LABEL_SIZE) {	
		operand_ofset[j] = *end_of_word;
		end_of_word++;
		j++;
	}

	if (j == MAX_LABEL_SIZE+1 || *end_of_word!='}') {	
		printf("error at line number %d ilegal operand after %s", line_num,
				instruction_list[i].instruction);
		return 0;
	}

	operand_ofset[j] = '\0';
	end_of_word++;
	return 1;
}

int update_operand(char *operand,char *operand_ofset,int work_on_source) {
	/*index_addressing*/
	if (*operand_ofset!='\0') {
		if (work_on_source==1) {
			line_data->line_word.inst.source_addressing=2;
			if (*operand_ofset=='r' && operand_ofset[1]>='0' && operand_ofset[1]<='7'
			    && strlen(operand_ofset)==2)
				line_data->line_word.inst.source_register=operand_ofset[1]-'0';
			
		}
		else {
			line_data->line_word.inst.destination_addressing=2;
			if (*operand_ofset=='r' && operand_ofset[1]>='0' && operand_ofset[1]<='7'
			    && strlen(operand_ofset)==2)
				line_data->line_word.inst.destination_register=operand_ofset[1]-'0';
		}
		return 0;
	}

	/*register addressing*/
	if (*operand=='r' && operand[1]>='0' && operand[1]<='7' && strlen(operand)==2) {
		if (work_on_source==1) {
			line_data->line_word.inst.source_addressing=3;
			line_data->line_word.inst.source_register=operand[1]-'0';
		}
		else {
			line_data->line_word.inst.destination_addressing=3;
			line_data->line_word.inst.destination_register=operand[1]-'0';
		}
		return 0;
	}

	/*direct addressing*/
	if (*operand!='#') {
		if (work_on_source==1) 
			line_data->line_word.inst.source_addressing=1;
		else 
			line_data->line_word.inst.destination_addressing=1;
		return 0;
	}
	/*for imediat addressing it is allready 0*/
	return 0;
}

int add_operand_lines (char *operand,char *operand_ofset,int work_on_source,int i,int line_num) {
	int addressing;

	if (work_on_source)
		addressing=line_data->line_word.inst.source_addressing;
	else
		addressing=line_data->line_word.inst.destination_addressing;

	switch (addressing) {

		case 0: 
			if ((!instruction_list[i].source_imidiat_addressing && work_on_source)
			    || (!instruction_list[i].destination_imidiat_addressing && !work_on_source)) {
				printf("\nerror at line number %d ilegal addressing\n", line_num);	
				return 0;
			}
			line_data = New(line_parse);
			line_data->decimal_address = IC;
			line_data->label_to_extract = NULL;
			lines_data_list = list_append(lines_data_list, line_data);
			IC++;

			if((line_data->line_word.data.data=extract_number(&operand[1], line_num))==999999)
				return 0;
			break;

		case 1:
			if ((!instruction_list[i].source_direct_addressing && work_on_source)
			    || (!instruction_list[i].destination_direct_addressing && !work_on_source)) {
				printf("\nerror at line number %d ilegal addressing\n", line_num);	
				return 0;
			}
			line_data = New(line_parse);
			line_data->decimal_address = IC;
			line_data->line_word.data.data=0;
			lines_data_list = list_append(lines_data_list, line_data);
			IC++;
			line_data->label_to_extract=(char *)malloc(strlen(operand)+1);
			strcpy(line_data->label_to_extract,operand);
			break;

		case 2:
			if ((!instruction_list[i].source_index_addressing && work_on_source)
			    || (!instruction_list[i].destination_index_addressing && !work_on_source)) {
				printf("\nerror at line number %d ilegal addressing\n", line_num);	
				return 0;
			}
			line_data = New(line_parse);
			line_data->decimal_address = IC;
			line_data->line_word.data.data=0;
			lines_data_list = list_append(lines_data_list, line_data);
			IC++;
			line_data->label_to_extract=(char *)malloc(strlen(operand)+1);
			strcpy(line_data->label_to_extract,operand);

			if (isdigit(*operand_ofset)) {
				line_data = New(line_parse);
				line_data->decimal_address = IC;
				line_data->label_to_extract = NULL;
				lines_data_list = list_append(lines_data_list, line_data);
				IC++;

				if((line_data->line_word.data.data=extract_number(operand_ofset, line_num))==999999) {
					printf("\nerror at line number %d ilegal addressing\n", line_num);	
					return 0;
				}
			}
			else if (!(*operand_ofset=='r' && strlen(operand_ofset)==2 &&
				*(operand_ofset+1)>='0' && *(operand_ofset+1)<='7')) {
						
				line_data = New(line_parse);
				line_data->decimal_address = IC;
				line_data->line_word.data.data=0;
				lines_data_list = list_append(lines_data_list, line_data);
				IC++;
				line_data->label_to_extract=(char *)malloc(strlen(operand_ofset)+1);
				strcpy(line_data->label_to_extract,operand_ofset);

			}

			break;

		case 3:
			if  ((!instruction_list[i].source_direct_register_addressing && work_on_source)
			    || (!instruction_list[i].destination_direct_register_addressing && !work_on_source)) {
				printf("\nerror at line number %d ilegal addressing\n", line_num);	
				return 0;
			}
			break;
		}

return 1;
}
