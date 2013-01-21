#include "error.h"
#include "reader.h"
#include "parser.h"
#include <stdlib.h>

/**
 * Main is responsible for initiating the file reading and parsing.
 */
int main(int argc, char* argv[]) {
	int i;
	
	if (argc < 2)
		error_fatal(ErrorMissingArgument);
	
	/* Open and read files. */
	for (i = 1; i < argc; i++) {
		reader_open_file(argv[i]);

		parser_parse();

		reader_close_file();
	}

	return EXIT_SUCCESS;
}
