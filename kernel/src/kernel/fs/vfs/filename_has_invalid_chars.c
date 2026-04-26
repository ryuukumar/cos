#include <kclib/ctype.h>
#include <kernel/fs/vfs.h>

bool filename_has_invalid_chars (char* filename) {
	while (*filename != 0) {
		unsigned char c = (unsigned char)*filename;
		if (c == '/' || !isprint (c)) return true;
		filename++;
	}
	return false;
}