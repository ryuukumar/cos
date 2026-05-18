/*
 * ctype.c
 * Copyright (C) 2026  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, see <https://www.gnu.org/licenses/>.
 */

#include <kclib/ctype.h>

/*!
 * Checks if a character is alphanumeric
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int isalnum (int c) { return isalpha (c) || isdigit (c); }

/*!
 * Checks if a character is alphabetic
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int isalpha (int c) { return islower (c) || isupper (c); }

/*!
 * Checks if a character is lowercase
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int islower (int c) { return 'a' <= c && c <= 'z'; }

/*!
 * Checks if a character is an uppercase character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int isupper (int c) { return 'A' <= c && c <= 'Z'; }

/*!
 * Checks if a character is a digit
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int isdigit (int c) { return '0' <= c && c <= '9'; }

/*!
 * Checks if a character is a hexadecimal character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int isxdigit (int c) { return isdigit (c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'); }

/*!
 * Checks if a character is a control character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int iscntrl (int c) { return c == 127 || (0 <= c && c <= 31); }

/*!
 * Checks if a character is a graphical character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int isgraph (int c) { return 33 <= c && c <= 126; }

/*!
 * Checks if a character is a space character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int isspace (int c) { return c == ' ' || (9 <= c && c <= 13); }

/*!
 * Checks if a character is a blank character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int isblank (int c) { return c == '\t' || c == ' '; }

/*!
 * Checks if a character is a printing character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int isprint (int c) { return isgraph (c) || c == ' '; }

/*!
 * Checks if a character is a punctuation character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
int ispunct (int c) { return isgraph (c) && !isalnum (c); }

/*!
 * Converts a character to lowercase
 * @param c character to assess
 * @return lowercase character if matches, else c
 */
int tolower (int c) { return isupper (c) ? (c + ('a' - 'A')) : c; }

/*!
 * Converts a character to uppercase
 * @param c character to assess
 * @return uppercase character if matches, else c
 */
int toupper (int c) { return islower (c) ? (c - ('a' - 'A')) : c; }