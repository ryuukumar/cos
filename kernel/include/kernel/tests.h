#ifndef TESTS_H
#define TESTS_H

typedef bool (*test_t) (void);
typedef struct reg_test_t reg_test_t;

struct reg_test_t {
	test_t		handler;
	const char* description;
	reg_test_t* next;
};

void register_test (test_t t_handler, const char* t_failmessage);
void run_tests (void);

#endif