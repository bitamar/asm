#include "parser.h"
#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parser_parse() {
	char* line;
	int ic = 0, dc = 0;
	short int label;

	while ((line = reader_get_line())) {
		label = parser_line_get_label(line);
		printf("%s:%d\n", line, label);
		free(line);
	}
}

short int parser_line_get_label(const char* line) {
	short int i;
	const char* labels[] = {"MAIN", "LOOP", "END", "STR", "LENGTH", "K"};

	for (i = 0; i < ParserLabelsAmount; i++){
		if (parser_line_has_label(line, labels[i])) {
			return 100;
		}
	}

	return ParserNoLabel;
}

short int parser_line_has_label(const char* line, const char* label) {
	/* Iterate the first word in the line, until colon, space or end of line is
	 * found. */
	while(*line && *line != ':' && *line != ' ') {
		/* Return false when a character differs between the word and the
		 * string. */
		if (*line != *label)
			return 0;

		line++;
		label++;
	}

	return 1;
}
