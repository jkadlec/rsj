#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <tcmalloc.h>
#include <ck_spinlock.h>

#include "worker.h"
#include "table.h"
#include "init.h"
#include "debug.h"

//from stackoverflow
void stick_this_thread_to_core(int core_id) {
        int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
        dbg_threading("CORES: %d\n", num_cores);
        if (core_id >= num_cores) {
                dbg_threading("not setting affinity\n");
                return;
        }

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core_id, &cpuset);

        pthread_t current_thread = pthread_self();    
        pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
        pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
        for (int j = 0; j < num_cores; j++) {
                if (CPU_ISSET(j, &cpuset)) {
                        dbg_threading("Thread %d out of %d set.\n", j, num_cores);
                }
        }
}

int int_comparison(const void *rb_a, const void *rb_b,
                   void *rb_param)
{
	int a = ((rsj_pair_t *)rb_a)->price_bid;
	int b = ((rsj_pair_t *)rb_b)->price_bid;
	if (a > b) {
		return 1;
	} else if (a < b) {
		return -1;
	} else {
		return 0;
	}
}

void initialize_c_style()
{
	stick_this_thread_to_core(0);
	global_context = (context_t *)malloc(sizeof(context_t));
        assert(global_context != NULL);
	/* Init basic variables. */
	global_context->global_id = 0;
	global_context->running = 1;
	global_context->order_index = 0;
	int ret = pthread_barrier_init(&start_barrier, NULL,
	                               THREAD_COUNT + 1);
	assert(ret == 0);

	/* Init histories. */	
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
		global_context->fp_i_history[i] = (double *)malloc(HISTORY_SIZE * sizeof(double));
		for (int j = 0; j < HISTORY_SIZE; j++) {
			global_context->fp_i_history[i][j] = 0.0;
		}
		global_context->order_indices_bid[i] = 0;
		assert(global_context->fp_i_history[i]);
	}
	
	/* Start worker threads. */
	for (int i = 0; i < THREAD_COUNT; i++) {
		pthread_create(&(global_context->worker_threads[i]),
		               NULL, worker_start, global_context);
	}
	
	dbg_threading("Threads started.\n");
	
	/* Init lookup structures + price histories. */
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
		global_context->asks[i].tree = rb_create(int_comparison, NULL,
		                                     &rb_allocator_default);
		assert(global_context->asks[i].tree);
		global_context->asks[i].vol_ask = 0;
		global_context->asks[i].price_ask = INT_MAX;
		global_context->bids[i].tree = rb_create(int_comparison, NULL,
		                                     &rb_allocator_default);
		assert(global_context->bids[i].tree);
		for (int j = 0; j < HISTORY_SIZE; j++) {
			global_context->bids[i].history[j].price_bid = 0;
			global_context->bids[i].history[j].vol_bid = 0;
			global_context->asks[i].history[j] = 0;
		}
		global_context->bids[i].price_bid = 0;
		global_context->bids[i].vol_bid = 0;
	}

#ifdef ATOMIC
	for (int i = 0; i < HISTORY_SIZE; i++) {
		global_context->order[i] = 0;
		global_context->order_sum[i] = 0;
		global_context->sum_history[i] = 0.0;
	}

	/* Ordering. */	
	global_context->order[HISTORY_SIZE - 1] = 1;
	global_context->order_sum[HISTORY_SIZE - 1] = 1;
#endif

#ifdef SPIN
	for (int i = 0; i < HISTORY_SIZE; i++) {
//		global_context->order[i] = (lock_t *)malloc(sizeof(lock_t));
//		global_context->order_sum[i] = (lock_t *)malloc(sizeof(lock_t));
#ifdef NORMAL
		lock_init(global_context->order[i], PTHREAD_PROCESS_SHARED);
		lock_init(global_context->order_sum[i], PTHREAD_PROCESS_SHARED);
#else
		global_context->order[i] = CK_SPINLOCK_FAS_INITIALIZER;
		global_context->order_sum[i] = CK_SPINLOCK_FAS_INITIALIZER;
#endif
		lock_lock(&global_context->order[i]);
		lock_lock(&global_context->order_sum[i]);
		global_context->sum_history[i] = 0.0;
	}

	lock_unlock(&global_context->order[HISTORY_SIZE - 1]);
	lock_unlock(&global_context->order_sum[HISTORY_SIZE - 1]);
#endif
	
	/* Ring buffer. */
	global_context->buffer = (ck_ring_t *)malloc(sizeof(ck_ring_t));
	void *data_buffer = malloc(BUFFER_SIZE * sizeof(void *));
	ck_ring_init(global_context->buffer, data_buffer, BUFFER_SIZE);
	
	dbg_threading("Structures allocated. (context=%p)\n",
	              global_context);
	
	pthread_barrier_wait(&start_barrier);
}
