#include <sys/types.h>
#include <arch/x86_64-cos/unistd.h>
#include <errno.h>

void* sbrk (ptrdiff_t incr) {
    static char *heap_end = 0;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = (char *)brk(0); 
    }

    prev_heap_end = heap_end;
    
    if (brk(heap_end + incr) < 0) {
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end += incr;
    return (void *)prev_heap_end;
}