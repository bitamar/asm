#include "error.h"
#include "list.h"
#include "parser.h"
#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List parser_symbols_list;

void parser_parse() {
	const struct instructions instruction_list[] = {
		{0, 2, "mov"},
		{1, 2, "cmp"},
		{2, 2, "add"},
		{3, 2, "sub"},
		{6, 2, "lea"},
		{4, 1, "not"},
		{5, 1, "clr"},
		{7, 1, "inc"},
		{8, 1, "dec"},
		{9, 1, "jmp"},
		{10, 1, "bne"},
		{11, 1, "red"},
		{12, 1, "prn"},
		{13, 1, "jsr"},
		{14, 0, "rts"},
		{15, 0, "stop"}
	};
	
	char* line;
	char* label;
	char *begin_of_word,*end_of_word;
	int ic = 0, dc = 0, line_length, line_num = 0, line_num_ofset = 0,i;
	
	
	while ((line = reader_get_line())) {
		line_num++;
		
		/* this is for remark line */
		if (*line==';') 
			continue;

		line_length=strlen(line);
		label = parser_get_label(line);
		
		begin_of_word=line;
		/* begin_of_word points to first char after label */
		if (label)
			begin_of_word=line+strlen(label)+1; 

		while (*begin_of_word==' ' || *begin_of_word=='\t')
			begin_of_word++;
		/* this is an empty line */
		if (*begin_of_word=='\0' && !label) 
			continue;
		
		line_num_ofset++;		

		/* find end of command */
		end_of_word=begin_of_word+1;
		while (*end_of_word!=' ' && *end_of_word!='\t' && *end_of_word!='/' && *end_of_word!='\0')
			end_of_word++;
		
		/* this is a data line */
		if (!strncmp(begin_of_word,".data",end_of_word-begin_of_word-1) && *end_of_word!='/') {

			printf("\nthis is a data line\n");

	/*		while (*end_of_word!='\0') {
				while (*end_of_word=' ' or *end_of_word='\t')
					end_of_word++;
				begin_of_word=end_of_word;

				if (*begin_of_word='\0')
					continue;

				while (*end_of_word!=' ' && *end_of_word!='\t' && *end_of_word!='\0' *end_of_word!=',')
					end_of_word++; */
				
				
			continue;
		}

		/* this is a string line */
		if (!strncmp(begin_of_word,".string",end_of_word-begin_of_word-1) && *end_of_word!='/') {
			
			begin_of_word=end_of_word;

			while (*begin_of_word==' ' || *begin_of_word=='\t')
				begin_of_word++;

			if (*begin_of_word!='"') {
				printf ("\n%d error, string expected after .string\n",line_num);
				continue;
			}
			
			end_of_word=begin_of_word+1;
			while (*end_of_word!=' ' && *end_of_word!='\t' && *end_of_word!='\0')
				end_of_word++;
			if (*(end_of_word-1)!='"') {
				printf ("\n%d error, string expected after .string\n",line_num);
				continue;
			}

			printf("\nthis is a string line, the string is %s\n",begin_of_word);

			while (*end_of_word==' ' || *end_of_word=='\t' && *end_of_word!='\0')
				end_of_word++;	

			if (*end_of_word!='\0')	
				printf("%d worning, additional unused charecters at end of line",line_num);
			continue;
		}
		
		for (i=0;i<16 && strncmp(begin_of_word,instruction_list[i].instruction,end_of_word-begin_of_word-1);i++);
		if (i==16) {
			printf("\nerror in line %d unknown instruction\n",line_num);
			continue;
		}
	
				

		printf("\n%d label is %s command is %s line is %s\n",line_num, label,instruction_list[i].instruction,line);
		free(label);
		free(line);
	}
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
	if (!(*c >= 'a' && *c <= 'z') && !(*c >= 'A' && *c <= 'Z')) {
		free(label);
		return NULL;
	}
		

	while(((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') || 
		(*c >= '0' && *c <= '9')) && len <= MAX_LABEL_SIZE + 1) {
		c++;
		len++;
	}
	
	if (line[len] == ':') {
		strncpy(label, line, len);
		label[len] = '\0';
		return label;
	}
	
	free(label);
	return NULL;
}
