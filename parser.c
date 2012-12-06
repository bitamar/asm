#include "parser.h"
#include "reader.h"
#include <stdio.h>
#include <stdlib.h>

/**
 *
 */
void parser_parse() {
	char* line;
	while ((line = reader_get_line())) {
		printf("%s\n", line);
		free(line);
	}
}
