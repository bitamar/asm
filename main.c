#include "error.h"
#include "reader.h"
#include "parser.h"
#include <stdlib.h>

/**
 * Main is responsible for initiating the file reading and parsing.
 */
int main(int argc, char* argv[]) {
	if (argc < 2)
		error_fatal(ERROR_MISSING_ARGUMENT);

	reader_open_file(argv[1]);

	parser_parse();

	reader_close_file();

	return EXIT_SUCCESS;
}
