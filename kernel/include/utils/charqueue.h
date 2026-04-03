#ifndef CHARQUEUE_H
#define CHARQUEUE_H

#include <kernel/memmgt.h>

typedef struct charqueue_page_t charqueue_page_t;

struct charqueue_page_t {
	charqueue_page_t* next;
	unsigned char	  data[PAGE_SIZE - sizeof (charqueue_page_t*)];
};

typedef struct {
	charqueue_page_t* current_page;
	uint64_t		  offset;
} charqueue_ptr_t;

typedef struct {
	charqueue_ptr_t head, tail;
	bool			lock;
} charqueue;

charqueue* create_charqueue (void);
int		   free_charqueue (charqueue* queue);

int push_charqueue (charqueue* queue, unsigned char insert);
int pop_charqueue (charqueue* queue, unsigned char* ret);
int peek_charqueue (charqueue* queue, unsigned char* ret);

#endif