#pragma once

#include <stddef.h>

int dispatch_builtin (size_t argc, char** argv);

int builtin_chdir (int argc, char** argv);
int builtin_echo (int argc, char** argv);
int builtin_eval (int argc, char** argv);
int builtin_exit (int argc, char** argv);
int builtin_getpid (int argc, char** argv);
int builtin_ls (int argc, char** argv);
int builtin_mkdir (int argc, char** argv);
int builtin_pwd (int argc, char** argv);
int builtin_source (int argc, char** argv);
int builtin_stat (int argc, char** argv);
int builtin_test (int argc, char** argv);
int builtin_touch (int argc, char** argv);
