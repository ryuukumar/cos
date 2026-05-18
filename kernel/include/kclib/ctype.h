/*
 * ctype.h
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

#pragma once

int isalnum (int c);
int isalpha (int c);
int islower (int c);
int isupper (int c);
int isdigit (int c);
int isxdigit (int c);
int iscntrl (int c);
int isgraph (int c);
int isspace (int c);
int isblank (int c);
int isprint (int c);
int ispunct (int c);

int tolower (int c);
int toupper (int c);
