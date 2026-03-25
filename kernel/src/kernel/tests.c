#include <kernel/serial.h>
#include <kernel/tests.h>
#include <stdint.h>

reg_test_t* tests_head = nullptr;
reg_test_t* tests_tail = nullptr;
uint64_t	test_count = 0;

void register_test (test_t t_handler, const char* t_failmessage);

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
}