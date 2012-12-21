#include "parser.h"
#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 *
 */
void parser_parse() {
	char* line;
	int ic = 0, dc = 0;


	while ((line = reader_get_line())) {

		printf("%s\n", line);
		free(line);
	}
}

char parser_line_get_label(const char* line) {
	char* labels[] = {};
}
