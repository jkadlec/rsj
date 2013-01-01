#ifndef HELPERS_H
#define HELPERS_H

#include "structures.h"

#define unlikely(x) __builtin_expect(!!(x), 0)

inline unsigned int prev_index(unsigned int index)
{
	return (index - 1) % HISTORY_SIZE;
}

inline unsigned int next_index(unsigned int index)
{
	return (index + 1) % HISTORY_SIZE;
}

inline unsigned int prev_index_spec(unsigned int index)
{
	return (unsigned int)(index - 1) % SPECIFIC_HISTORY_SIZE;
}

#endif // HELPERS_H
