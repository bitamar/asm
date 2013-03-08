#include "error.h"
#include "reader.h"
#include "translator.h"
#include "utils.h"
#include <stdlib.h>

/* Holds the three output files; ext, ent & ob. */
FILE* _translate_output_files[3];

/**
 * Callback function for list_foreach; Performs the translation for a single
 * line.
 *
 * @param _line_data
 *   A LineData object as a void pointer (For keeping the list generic).
 */
void _translate_line(void* _line_data) {
	LineData* line_data = _line_data;
	Label* label_found = NULL;
	/* Used for comparing two labels using _parser_compare_labels. */
	Label* dummy_label;
	long address;
	/* The final code we print in ob. */
	long code;
	char code_buffer[CODE_SIZE + 1];

	/* If the line has a related label, search for it in the symbols list for
	 * setting its address in the line. */
	if (line_data->label_to_extract) {
		NewLabel(dummy_label);
		dummy_label->label = line_data->label_to_extract;
		label_found = list_find_item(parser_data.parser_symbols, dummy_label, &_parser_compare_labels);
		free(dummy_label);

		/* Determine line type. */
		line_data->are = label_found && label_found->label_type == LABEL_TYPE_EXTERN ? 'e' : 'r';

		if (!label_found) {
			fprintf(stderr, "Error: Label \"%s\" not found\n", line_data->label_to_extract);
			parser_data.errors++;
		}
		else if (label_found->line) {
			/* Add the 100 offset, Actually 99 because the first line is 1. */
			line_data->line_word.data = label_found->line + LINE_OFFSET - 1;
			/* If it's a data line, add the instructions count to its data. */
			if (line_data->is_instruction || label_found->label_type == LABEL_TYPE_DATA)
				line_data->line_word.data += parser_data.IC;
		}
		/* Write external labels the ext file. */
		else if (label_found->label_type == LABEL_TYPE_EXTERN && !line_data->is_instruction)
			fprintf(_translate_output_files[EXT_FILE], "%s\t%d\n", label_found->label, base4(line_data->decimal_address + LINE_OFFSET - 1));
	}

	code = line_data->line_word.data;
	if (line_data->is_instruction) {
		/* When the line is an instruction line, rebulid the code using the
		 * bits stored under inst, to make sure the code is correct under any
		 * machine. */
		code = line_data->line_word.inst.comb; /* *1 = 2^0 */
		code += line_data->line_word.inst.dest_reg * 4; /* = 2^2 */
		code += line_data->line_word.inst.dest_address * 32; /* = 2^5 */
		code += line_data->line_word.inst.src_reg * 128; /* = 2^7 */
		code += line_data->line_word.inst.src_address * 1024; /* = 2^10 */
		code += line_data->line_word.inst.opcode * 4096; /* = 2^12 */
		code += line_data->line_word.inst.type * 65536; /* = 2^16 */
	}

	/* Avoid adding the offset when the address is zero. */
	address = line_data->decimal_address + LINE_OFFSET - 1;

	/* If it's a data line, add the instructions count to its address. */
	if (!line_data->are)
		address += parser_data.IC;

	/* Print a line to the ob file. */
	fprintf(_translate_output_files[OB_FILE], "%d\t\t\t\t%s\t\t%c\n", base4(address), base4code(code, code_buffer), line_data->are);
}

/**
 * Translate the lines; Creates the ext and ob file and then iterating the
 * commands and data lists calling _translate_line for each line.
 */
void _translate_lines() {
	OpenFile(_translate_output_files[EXT_FILE], "ext");
	OpenFile(_translate_output_files[OB_FILE], "ob");

	/* Print the ob file title. */
	fprintf(_translate_output_files[OB_FILE], "\t\t\t\t\t%d\t%d\n", base4(parser_data.IC), base4(parser_data.DC));
	/* Translate the lists. */
	list_foreach(parser_data.commands_list, &_translate_line);
	list_foreach(parser_data.data_list, &_translate_line);

	fclose(_translate_output_files[EXT_FILE]);
	fclose(_translate_output_files[OB_FILE]);
}

/**
 * Call-back function for list_foreach; Write a label to the entry
 * file.
 *
 * param _label
 *   Label object converted to void pointer.
 */
void _translate_entry_label(void* _label) {
	unsigned long address;
	Label* label = _label;
	Label* label_found;

	/* Search for the label in the symbols list. */
	label_found = list_find_item(parser_data.parser_symbols, label, &_parser_compare_labels);
	if (!label_found) {
		fprintf(stderr, "Error in %s: Entry label not defined.\n", reader_get_file_name(ReaderFileExtension));
		parser_data.errors++;
		return;
	}
	/* Set the real address by adding 99. */
	address = label_found->line + LINE_OFFSET - 1;
	/* Add the instructions count when the label is on a data line. */
	if (label_found->label_type == LABEL_TYPE_DATA)
		address += parser_data.IC;

	/* Print a line to the entry file. */
	fprintf(_translate_output_files[ENT_FILE], "%s\t%d\n", label_found->label, base4(address));
}

/**
 * Handles the entry file creation; Calls _translate_entry_label for each
 * entry symbol.
 */
void _translate_create_ent_file() {
	OpenFile(_translate_output_files[ENT_FILE], "ent");
	list_foreach(parser_data.parser_entry_symbols, &_translate_entry_label);
	fclose(_translate_output_files[ENT_FILE]);
}

/**
 * Perform the "Second phase" translation.
 */
int translate() {
	_translate_create_ent_file();
	_translate_lines();
	return !parser_data.errors;
}
