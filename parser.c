#include "error.h"
#include "parser.h"
#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	char label[MAX_LABEL_SIZE + 1];
	int ic = 0, dc = 0, line_length, line_num = 0, line_num_ofset = 0;
	
	
	while ((line = reader_get_line())) {
		line_num++;
		line_length=strlen(line);
		strcpy(label,parser_get_label(line));
		printf("\n%s\n%s\n", label, line);
		
		
		free(line);
	}
}

char* parser_get_label(const char* line) {
	int len = 0;
	char label[MAX_LABEL_SIZE + 1];
	char* c = line;
	
	/* Iterate the first word in the line, until colon, space or end of line is
	 * found. */
	if (!(*c >= 'a' && *c <= 'z') && !(*c >= 'A' && *c <= 'Z'))
		return NULL;

	while(((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z')) && len <= MAX_LABEL_SIZE + 1) {
		c++;
		len++;
	}
	
	if (line[len] == ':') {
		strncpy(label, line, len);
		label[len] = '\0';
		return label;
	}
	
	return NULL;
}
