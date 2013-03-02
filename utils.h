/*
 * utils.h
 */

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
char* base4code(long decimal, char* buffer);

#endif /* UTILS_H_ */
