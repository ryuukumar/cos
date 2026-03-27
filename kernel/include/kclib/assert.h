#ifndef ASSERT_H
#define ASSERT_H

typedef bool (*kassert_handler) (bool, const char*);

bool kassert_halt (bool condition);
bool kassert_handle (bool condition, const char* message, kassert_handler handler);

// Requires serial to be initialised!

bool kassert_print_halt (bool condition, const char* message);
bool kassert_print_no_block (bool condition, const char* message);
bool kassert_print_handle (bool condition, const char* message, kassert_handler handler);

#endif