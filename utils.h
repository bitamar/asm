#ifndef UTILS_H_
#define UTILS_H_

#define CODE_SIZE 10
/**
 * Convert a decimal number to base 4.
 *
 * @param decimal
 *   Decimal number.
 *
 * @return
 *   The given number converted to base 4.
 */
int base4(int decimal);

/**
 * Convert a decimal number to a ten bits string.
 *
 * @param decimal
 *   The number to convert.
 * @param buffer
 *   A buffer to save the string to.
 */
void base4code(long decimal, char* buffer);

/**
 * Check if a character is either a space or a tab.
 *
 * @param c
 *   Character to check
 * @return
 *   Non-zero if true.
 */
#define IsBlank(c) ((c) == ' ' || (c) == '\t')

/**
 * Advance a string to the beginning of the next word.
 */
#define NextWord(x) while (IsBlank(*(x))) (x)++

#endif /* UTILS_H_ */
