#include <ctype.h>
#include <gen_argv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* get_next_arg (char* arg) {
	while (arg && isspace ((unsigned char)*arg))
		arg++;
	if (arg && *arg != 0) return (char*)arg;
	return nullptr;
}

static size_t get_full_arglen (char* arg, size_t* arglen_parsed) {
	size_t ret_arglen_parsed = 0;
	if (!arg) return 0;

	char* start = arg;
	bool  in_singlequotes = *arg == '\'', in_doublequotes = *arg == '\"';
	if (in_singlequotes) {
		arg++;
		do {
			if (*arg == '\\') arg++;
			if (*arg == 0) break;
			arg++;
			ret_arglen_parsed++;
		} while (*arg != '\'');
		arg++;
	} else if (in_doublequotes) {
		arg++;
		do {
			if (*arg == '\\') arg++;
			if (*arg == 0) break;
			arg++;
			ret_arglen_parsed++;
		} while (*arg != '\"');
		arg++;
	} else {
		do {
			if (*arg == '\\') arg++;
			if (*arg == 0) break;
			arg++;
			ret_arglen_parsed++;
		} while (!isspace ((unsigned char)*arg));
	}

	if (arglen_parsed) *arglen_parsed = ret_arglen_parsed;
	return arg - start;
}

static char parse_escape_char (char c) {
	switch (c) {
	case 'n':
		return '\n';
	case 't':
		return '\t';
	case 'b':
		return '\b';
	case 'r':
		return '\r';
	case '\\':
		return '\\';
	case '\'':
		return '\'';
	case '\"':
		return '\"';
	case '0':
		return '\0';
	default:
		return c;
	}
}

static char* parse_next_arg (char* arg) {
	size_t parsed_len = 0;
	get_full_arglen (arg, &parsed_len);

	char* ptr = malloc ((parsed_len + 1) * sizeof (char));
	char* ret = ptr;
	bool  in_singlequotes = *arg == '\'', in_doublequotes = *arg == '\"';
	if (in_singlequotes) {
		arg++;
		do {
			if (*arg == '\\') arg++;
			if (*arg == 0) break;
			*ptr++ = *arg;
			arg++;
		} while (*arg != '\'');
		arg++;
	} else if (in_doublequotes) {
		arg++;
		do {
			if (*arg == '\\') {
				arg++;
				if (*arg == 0) break;
				*ptr++ = parse_escape_char (*arg);
			} else {
				*ptr++ = *arg;
			}
			arg++;
		} while (*arg != '\"');
		arg++;
	} else {
		do {
			if (*arg == '\\') arg++;
			if (*arg == 0) break;
			*ptr++ = *arg;
			arg++;
		} while (!isspace ((unsigned char)*arg));
	}

	*ptr = 0;
	return ret;
}

char** gen_argv (const char* input, size_t* argc) {
	uint64_t arg_count = 0;
	char*	 nextarg = get_next_arg ((char*)input);
	while (nextarg) {
		size_t to_skip = get_full_arglen (nextarg, nullptr);
		nextarg += to_skip;
		nextarg = get_next_arg (nextarg);
		arg_count++;
	}

	char** ret_argv = malloc ((arg_count + 1) * sizeof (char*));
	memset (ret_argv, 0, (arg_count + 1) * sizeof (char*));

	nextarg = get_next_arg ((char*)input);
	for (uint64_t i = 0; i < arg_count && nextarg; i++) {
		ret_argv[i] = parse_next_arg (nextarg);
		size_t to_skip = get_full_arglen (nextarg, nullptr);
		nextarg += to_skip;
		nextarg = get_next_arg (nextarg);
	}

	printf ("Arguments received: ");
	for (uint64_t i = 0; i < arg_count; i++)
		printf ("\"%s\" ", ret_argv[i]);
	printf ("\n");

	*argc = arg_count;
	return ret_argv;
}