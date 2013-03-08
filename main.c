/**
 * Assembler for systems programming course.
 * by Sami Dadon and Itamar Bar-Lev.
 */

#include "error.h"
#include "parser.h"
#include "reader.h"
#include "translator.h"
#include <stdlib.h>

/*
 * Main is responsible for initiating the file reading and parsing.
 */
int main(int argc, char* argv[]) {
	int success;
	
	if (argc < 2)
		error_fatal(ErrorMissingArgument);
	
	/* Open and read files. */
	for (; argc > 1; argc--) {
		reader_open_file(argv[argc - 1]);
		/* Build the data. */
		success = parse();
		reader_close_file();

		if (!success) {
			printf("Errors found, skipping translation.\n");
			parser_clean();
			continue;
		}

		printf("Parsing finished successfully.\n");
		/* Perform "Second iteration". */
		success = translate();

		if (success)
			printf("File assembled successfully.\n");
		else
			printf("Errors encountered in the translation phase.\n");

		/* Clean data. */
		parser_clean();
	}

	return EXIT_SUCCESS;
}
