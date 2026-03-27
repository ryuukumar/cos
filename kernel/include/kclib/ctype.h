#ifndef CTYPE_H
#define CTYPE_H

int isalnum (char c);
int isalpha (char c);
int islower (char c);
int isupper (char c);
int isdigit (char c);
int isxdigit (char c);
int iscntrl (char c);
int isgraph (char c);
int isspace (char c);
int isblank (char c);
int isprint (char c);
int ispunct (char c);

char tolower (char c);
char toupper (char c);

#endif