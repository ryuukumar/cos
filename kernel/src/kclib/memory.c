
#include <kclib/string.h>

/*!
Copy memory chunk of size n.

@param  dest destination
@param  src source
@param  n width of data
*/
void* kmemcpy (void* dest, const void* src, size_t n) {
	uint8_t*	   pdest = (uint8_t*)dest;
	const uint8_t* psrc = (const uint8_t*)src;

	for (size_t i = 0; i < n; i++)
		pdest[i] = psrc[i];

	return dest;
}

/*!
Set memory chunk of size n to be value c.

@param  s pointer to memory
@param  c value to set memory to
@param  n size of memory chunk
*/
void* kmemset (void* s, int c, size_t n) {
	uint8_t* p = (uint8_t*)s;

	for (size_t i = 0; i < n; i++)
		p[i] = (uint8_t)c;

	return s;
}

/*!
Move memory chunk.

@param  dest destination
@param  src source
@param  n width of data
*/
void* kmemmove (void* dest, const void* src, size_t n) {
	uint8_t*	   pdest = (uint8_t*)dest;
	const uint8_t* psrc = (const uint8_t*)src;

	if (src > dest)
		for (size_t i = 0; i < n; i++)
			pdest[i] = psrc[i];
	else if (src < dest)
		for (size_t i = n; i > 0; i--)
			pdest[i - 1] = psrc[i - 1];

	return dest;
}