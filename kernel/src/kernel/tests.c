#include <kassert.h>
#include <kernel/serial.h>
#include <kernel/tests.h>
#include <liballoc/liballoc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

reg_test_t* tests_head = nullptr;
reg_test_t* tests_tail = nullptr;
uint64_t	test_count = 0;

void register_test (test_t t_handler, const char* t_failmessage) {
	reg_test_t* new_test = kmalloc (sizeof (reg_test_t));
	kassert_print_halt (new_test != nullptr,
						"Tried to register a test but no memory was available.\n");

	new_test->handler = t_handler;
	new_test->description = strdup (t_failmessage);
	new_test->next = nullptr;

	if (tests_tail) tests_tail->next = new_test;

	tests_tail = new_test;
}

void run_tests (void) {
	uint64_t successful = 0, failed = 0;

	for (reg_test_t* test = tests_head; test != nullptr; test = test->next) {
		if (test->handler) {
			if (!test->handler ()) {
				write_serial_str ("Test failed: ");
				write_serial_str (test->description);
				write_serial ('\n');
				failed++;
			} else {
				write_serial_str ("Test succeeded: ");
				write_serial_str (test->description);
				write_serial ('\n');
				successful++;
			}
		}
	}

	printf ("Ran all tests! %lld succeeded, %lld failed.\n", successful, failed);
}