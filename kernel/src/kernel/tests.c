#include <kassert.h>
#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/tests.h>
#include <liballoc/liballoc.h>
#include <stdint.h>

reg_test_t* tests_head = nullptr;
reg_test_t* tests_tail = nullptr;
uint64_t	test_count = 0;

void register_test (test_t t_handler, const char* t_failmessage) {
	reg_test_t* new_test = kmalloc (sizeof (reg_test_t));
	kassert_print_halt (new_test != nullptr,
						"Tried to register a test but no memory was available.\n");

	new_test->handler = t_handler;
	new_test->description = kstrdup (t_failmessage);
	new_test->next = nullptr;

	if (tests_tail)
		tests_tail->next = new_test;
	else
		tests_head = new_test;

	tests_tail = new_test;
}

void run_tests (void) {
	uint64_t successful = 0, failed = 0;

	for (reg_test_t* test = tests_head; test != nullptr; test = test->next) {
		if (test->handler) {
			if (!test->handler ()) {
				kserial_printf ("Test failed: %s\n", test->description);
				failed++;
			} else {
				kserial_printf ("Test succeeded: %s\n", test->description);
				successful++;
			}
		}
	}

	kprintf ("Ran all tests! %lld succeeded, %lld failed.\n", successful, failed);
}