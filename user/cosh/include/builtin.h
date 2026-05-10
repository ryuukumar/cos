#pragma once

#include <stddef.h>

int dispatch_builtin (size_t argc, char** argv);

int builtin_chdir (int argc, char** argv);
int builtin_exit (int argc, char** argv);
int builtin_pwd (int argc, char** argv);
