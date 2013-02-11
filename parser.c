#include "error.h"
#include "parser.h"
#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parser_parse() {
	char* line;
	char* label;
	int ic = 0, dc = 0;
	

	while ((line = reader_get_line())) {
		label = parser_get_label(line);
		
		free(line);
		free(label);
	}
}

char* parser_get_label(const char* line) {
	int len = 0;
	char* label;
	const char* c = line;
	/* Iterate the first word in the line, until colon, space or end of line is
	 * found. */
	while(*c && *c != ' ' && *c != '\t') {
		c++;
		len++;
	}
	
	if (len && line[len - 1] == ':') {
		label = (char*)malloc(len - 1);
		if (!label)
			error_fatal(ErrorMemoryAlloc);
		
		strncpy(label, line, len - 1);
		label[len - 1] = '\0';
		return label;
	}
	
	return NULL;
}
