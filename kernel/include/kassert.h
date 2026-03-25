#ifndef KASSERT_H
#define KASSERT_H

typedef void (*kassert_handler) (bool, const char*);

void kassert_halt (bool condition);
void kassert_handle (bool condition, kassert_handler handler);

// Requires serial to be initialised!
void kassert_print_halt (bool condition, const char* message);
void kassert_print_no_block (bool condition, const char* message);
void kassert_print_handle (bool condition, const char* message, kassert_handler handler);

#endif