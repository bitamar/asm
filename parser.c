#include "error.h"
#include "list.h"
#include "parser.h"
#include "reader.h"
#include "char.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List parser_symbols_list;

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
	
	char* line;
	Label* label;
	char *begin_of_word,*end_of_word,command_type[7],source[80],destination[80];
	int line_num = 0, line_num_ofset = 0, i,j; 
	int num_of_param, num_of_comma,first_addressing_type,second_addressing_type;
	long data_number;
	
	
	while ((line = reader_get_line())) {		
		line_num++;
		
		/* this is for remark line */
		if (*line == ';') 
			continue;

		label = (Label*)malloc(sizeof(Label));
		label->label = parser_get_label(line);
		label->line = line_num_ofset;
		
		begin_of_word=line;
		
		if (label->label) {
			parser_symbols_list = list_add_ordered(parser_symbols_list, label, &_parser_compare_labels, &_parser_duplicated_label);
			/* begin_of_word points to first char after label */
			begin_of_word = line + strlen(label->label) + 1; 
		}
		else
			free(label);

		while (char_isblank(*begin_of_word))
			begin_of_word++;
		/* this is an empty line */
		if (*begin_of_word == '\0' && !label->label) 
			continue;
		
		line_num_ofset++;		

		/* find end of command */
		end_of_word = begin_of_word + 1;
		while (!char_isblank(*end_of_word) && *end_of_word != '/' && *end_of_word != '\0')
			end_of_word++;
		
		/* this is a data line */
		if (!strncmp(begin_of_word, ".data", 5) && (end_of_word - begin_of_word) == 5 && *end_of_word != '/') {
			num_of_param = 0;
			num_of_comma = 0;
			printf("\nthis is a data line\n");

			if (*end_of_word == '\0') 
				printf("\n%d warning .data line contains no data", line_num);

			while (*end_of_word != '\0') {
				find_next_non_blank_char(end_of_word);

				begin_of_word = end_of_word;
		
				if (*begin_of_word == '\0' && num_of_param == 0) 
					printf("\n%d warning .data line contain no data", line_num);
				
				if (*begin_of_word == '\0')
					continue;

				if (*begin_of_word != '-' && *begin_of_word != '+' && !isdigit(*begin_of_word)) {
					error_set("data line contain illegal number", line_num);
					break;
				}

				end_of_word++;
				data_number = 0;
				if (isdigit(*begin_of_word))
					data_number = *begin_of_word - '0';

				while (!char_isblank(*end_of_word) && *end_of_word != '\0' && *end_of_word != ',') {
					if (!isdigit(*end_of_word)) {						
						error_set("data line contain illegal number", line_num);;
						continue;
					}
					else {
						/* @TODO: Explain. */
						data_number = 10 * data_number + *end_of_word - '0';
						if ((*begin_of_word == '-' && data_number > -1 * MIN_DATA_NUMBER) || data_number > MAX_DATA_NUMBER) {
							error_set("number out of limit", line_num);
							while (!char_isblank(*end_of_word) && *end_of_word != '\0' && *end_of_word != ',')
								end_of_word++;
							
							continue;
						}
					}
					end_of_word++; 
				}

				if (*begin_of_word == '-')
					data_number *= -1;

				num_of_param++;

				if (data_number >= MIN_DATA_NUMBER && data_number <= MAX_DATA_NUMBER)
				printf("\n data is %ld\n", data_number);
			
				while (char_isblank(*end_of_word))
					end_of_word++;
				
				if (*end_of_word == ',') {
					end_of_word++;
					num_of_comma++;
				}
			}
			if (num_of_comma != num_of_param - 1)
				printf("\n%d warning data line contain spare comma at the end", line_num);
			continue;
		}

		/* this is a string line */
		
		if (!strncmp(begin_of_word, ".string", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/') {
			begin_of_word = end_of_word;

			while (char_isblank(*begin_of_word))
				begin_of_word++;

			if (*begin_of_word != '"') {
				error_set("string expected after .string", line_num);
				continue;
			}
			
			end_of_word = line + strlen(line) - 1;
			while (char_isblank(*end_of_word))
				end_of_word--;
			
			if (*(end_of_word) != '"' || end_of_word == begin_of_word) {
				error_set("string expected after .string", line_num);
				continue;
			}

		/*	printf("\nthis is a string line, the string is %s\n", begin_of_word);*/
			continue;
		}
		
		/* this is an entry label declaration line */
		if (!strncmp(begin_of_word, ".entry", 6) && (end_of_word - begin_of_word) == 6 && *end_of_word != '/') {
			begin_of_word=end_of_word;

			while (char_isblank(*begin_of_word))
				begin_of_word++;

			if (!isalpha(*begin_of_word)) {
				error_set("Not a legal label", line_num);
				continue;
			}
			
			end_of_word = line + strlen(line) - 1;
			while (char_isblank(*end_of_word))
				end_of_word--;
			
			while (isalnum(*end_of_word))
				end_of_word--; 
			
			if (end_of_word > begin_of_word) {
				error_set("Label expected after .entry", line_num);
				continue;
			}

			printf("this is an entry line, the entry label is %s", begin_of_word);
				continue;
		}

		/* this is an extern label declaration line */

		if (!strncmp(begin_of_word, ".extern", 7) && (end_of_word - begin_of_word) == 7 && *end_of_word != '/') {
			begin_of_word = end_of_word;

			while (char_isblank(*begin_of_word))
				begin_of_word++;

			if (!isalpha(*begin_of_word)) {
				error_set("not a legal label", line_num);
				continue;
			}
			
			end_of_word = line + strlen(line) - 1;
			while (char_isblank(*end_of_word))
				end_of_word--;
			
			while (isalnum(*end_of_word))
				end_of_word--; 
			
			if (end_of_word > begin_of_word) {
				error_set("label expected after .extern", line_num);
				continue;
			}

			printf("\nthis is an extern line, the extern label is %s\n", begin_of_word);
			continue;
		}
		
		/* this is a command line*/
		if (strlen(line)>80) {
			printf("%d error line length exiding 80 char",line_num);
			continue;
		}

		for (i=0; i < 16 && strncmp(begin_of_word, instruction_list[i].instruction, end_of_word - begin_of_word); i++);
		if (i == 16) {
			error_set("unknown instruction", line_num);
			continue;
		}

		/* find '/0' or /1 */
		find_next_non_blank_char(end_of_word);
		command_type[0]=*end_of_word;
		if (*end_of_word!='\0')	{
			end_of_word++;
			find_next_non_blank_char(end_of_word);			
			command_type[1]=*end_of_word;
			command_type[2]='\0';
		}
		if (!(command_type[0]=='/' && (command_type[1]=='0' || command_type[1]=='1'))) {
			
			printf("\n%d error, type epxpected after instruction\n",line_num);
			continue;
		}

		if (*end_of_word!='\0')	
			end_of_word++;

		if (command_type[1]=='1') {
			for (j=0;j<4;j++) {
				find_next_non_blank_char(end_of_word);
				command_type[2+j]=*end_of_word;
				command_type[2+j+1]='\0';
				if (*end_of_word!='\0')	
					end_of_word++;				
			}

			if (!(command_type[0]=='/' && command_type[2]=='/' && command_type[4]=='/' && 
				(command_type[5]=='0' || command_type[5]=='1') && 
				(command_type[3]=='0' || command_type[3]=='1'))) {
				printf("\n%d error, ilegal command type\n",line_num);
				continue;
			}
		}

		/* verify blank after type */
		if (!char_isblank(*end_of_word) && *end_of_word!='\0') {

			printf("\n%d error, a blank char is required after instruction\n",line_num);
			continue;
		}
		
		find_next_non_blank_char(end_of_word);

		if (*end_of_word=='#') {
			first_addressing_type=0;

			end_of_word++;

			begin_of_word = end_of_word;
		
			if (*begin_of_word != '-' && *begin_of_word != '+' && !isdigit(*begin_of_word)) {
				printf("\n%d error, ilegal opperand, expect number after #\n", line_num);
				continue;
			}
		
			end_of_word++;
			data_number = 0;
			if (isdigit(*begin_of_word))
				data_number = *begin_of_word - '0';
		
			while (!char_isblank(*end_of_word) && *end_of_word != '\0' && *end_of_word != ',') {
				if (!isdigit(*end_of_word)) {
					printf("%d line contain illegal number after #", line_num);
					continue;
				}
				else {
					
					data_number = 10 * data_number + *end_of_word - '0';
					if ((*begin_of_word == '-' && data_number > -1 * MIN_DATA_NUMBER) || data_number > MAX_DATA_NUMBER) {
					printf("%d number out of limit", line_num);
					while (!char_isblank(*end_of_word) && *end_of_word != '\0' && *end_of_word != ',')
						end_of_word++;
						continue;
					}
				}
				end_of_word++; 
			}

				if (*begin_of_word == '-')
					data_number *= -1;
		}



		printf("\n%d label is %s command is %s first parameter is %ld line is %s\n", line_num, label->label, instruction_list[i].instruction, data_number,line);
		free(line);
	}
	
	/*error_print_list();*/
	list_print(parser_symbols_list, &_parser_print_label);
}

char* parser_get_label(const char* line) {
	int len = 0;
	char* label;
	const char* c = line;
	
	label = (char*)malloc(MAX_LABEL_SIZE + 1);
	if (!label) 
		error_fatal(ErrorMemoryAlloc);
	
	/* Iterate the first word in the line, until colon, space or end of line is
	 * found. */
	if (!isalpha(*c)) {
		free(label);
		return NULL;
	}
		
	while(isalnum(*c) && len < MAX_LABEL_SIZE) {
		c++;
		len++;
	}
	
	if (line[len] == ':') {
		strncpy(label, line, len);
		label[len] = '\0';

		/* check if label name is register name*/
		if ((strlen(label)==2 && label[0]=='r' && (label[1]-'0')>=0 && (label[1]-'0')<=7) || 
		!strcmp(label,"PC") || !strcmp(label,"SP") || !strcmp(label,"PSW")) {
			printf("\n%s is ilegal label name, same as register\n",label);
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
	
	error_set("Redeclaring label.", label->line);
}

void _parser_print_label(void* data) {
	Label* label = data;
	printf("%d: %s\n", label->line, label->label);
}
