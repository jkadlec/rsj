#ifndef SYNC_H
#define SYNC_H

#include <ck_spinlock.h>

#include "globals.h"
#include "structures.h"

#ifdef SPIN
inline void wait_until_set_order(unsigned int index)
{
	while (lock_locked(&global_context->order[index])) {
		;
	}
//	lock_lock(global_context->order[index]);
//	lock_unlock(global_context->order[index]);
}

inline void wait_until_set_order_sum(unsigned int index)
{
	while (lock_locked(&global_context->order_sum[index])) {
		;
	}
//	lock_lock(global_context->order_sum[index]);
//	lock_unlock(global_context->order_sum[index]);
}

inline void sync_set_order(unsigned int index)
{
	lock_unlock(&global_context->order[index]);
}

inline void sync_unset_order(unsigned int index)
{
	lock_trylock(&global_context->order[index]);
}

inline void sync_set_order_sum(unsigned int index)
{
	lock_unlock(&global_context->order_sum[index]);
}

inline void sync_unset_order_sum(unsigned int index)
{
	lock_trylock(&global_context->order_sum[index]);
}

#endif //SPIN

#ifdef ATOMIC
/*
inline void wait_until_set_order(unsigned int index)
{
	while (global_context->order[index] == 0) {
		;
        }
}

inline void wait_until_set_order_sum(unsigned int index)
{
	while (global_context->order_sum[index] == 0) {
		;
        }
}

inline void sync_set_order(unsigned int index)
{
	global_context->order[index] = 1;
}

inline void sync_unset_order(unsigned int index)
{
	global_context->order[index] = 0;
}

inline void sync_set_order_sum(unsigned int index)
{
	global_context->order_sum[index] = 1;
}

inline void sync_unset_order_sum(unsigned int index)
{
	global_context->order_sum[index] = 0;
}

*/

inline void wait_until_set_order(unsigned int index)
{
	while (__sync_bool_compare_and_swap(&(global_context->order[index]),
	                                    0, 0)) {
		;
        }
}

inline void wait_until_set_order_sum(unsigned int index)
{
	while (__sync_bool_compare_and_swap(&(global_context->order_sum[index]),
	                                    0, 0)) {
		;
        }
}

inline void sync_set_order(unsigned int index)
{
	__sync_lock_test_and_set(&(global_context->order[index]), 1);
}

inline void sync_unset_order(unsigned int index)
{
	__sync_lock_test_and_set(&(global_context->order[index]), 0);
}

inline void sync_set_order_sum(unsigned int index)
{
	__sync_lock_test_and_set(&(global_context->order_sum[index]), 1);
}

inline void sync_unset_order_sum(unsigned int index)
{
	__sync_lock_test_and_set(&(global_context->order_sum[index]), 0);
}

#endif // ATOMIC

#endif // SYNC_H
