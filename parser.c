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
	short int label = ParserNoLabel, i;
	const char* labels[] = {"MAIN", "LOOP", "END", "STR", "LENGTH", "K"};

	for (i = 0; i < ParserLabelsAmount; i++){

	}

	return label;
}

short int parser_string_starts_with_word(const char* string, const char* word) {


	return 1;
}
