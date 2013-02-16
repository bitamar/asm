/* 
 * File:   char.h
 * Char utilities.
 */

#ifndef CHAR_H
#define	CHAR_H

/**
 * Check if a character is either a space or a tab.
 * 
 * @param c
 *   Character to check
 * @return 
 *   Non-zero if true.
 */
#define char_isblank(c) ((c) == ' ' || (c) == '\t')

#define find_next_non_blank_char(x) while (char_isblank(*(x))) (x)++

#endif	/* CHAR_H */
