#include "error.h"
#include "parser.h"
#include "reader.h"
#include "translator.h"
#include <stdlib.h>

/**
 * Assembler by Sami Dadon and Itamar Bar-Lev.
 *
 * Main is responsible for initiating the file reading and parsing.
 */
int main(int argc, char* argv[]) {
	int file_index;
	
	if (argc < 2)
		error_fatal(ErrorMissingArgument);
	
	/* Open and read files. */
	for (file_index = 1; file_index < argc; file_index++) {
		reader_open_file(argv[file_index]);
		/* Build the data. */
		parser_parse();
		reader_close_file();

		printf("Parsing finished.\n");

		/* Perform "Second iteration". */
		translate();

		printf("File %s assembled successfully.\n", reader_get_file_name(ReaderFileExtension));

		/* Clean data. */
		parser_clean();
	}

	return EXIT_SUCCESS;
}
