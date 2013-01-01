#include <stdlib.h>
#include <assert.h>

#include "worker.h"
#include "table.h"
#include "init.h"

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
	global_context = (context_t *)malloc(sizeof(context_t));
        assert(global_context != NULL);
	/* Init basic variables. */
	global_context->global_id = 0;
	global_context->running = 1;
	global_context->order_index = 0;
	int ret = pthread_barrier_init(&start_barrier, NULL,
	                               THREAD_COUNT + 1);
	assert(ret == 0);
	
	/* Init spinlocks. */
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
//		spinlock_init(&(global_context->insert_instrument_lock[i]),
//		              PTHREAD_PROCESS_SHARED);
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
	
	/* Init lookup structures. */
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
	
	for (int i = 0; i < HISTORY_SIZE; i++) {
		global_context->order[i] = 0;
		global_context->order_sum[i] = 0;
		global_context->sum_history[i] = 0.0;
	}
	
	global_context->order[HISTORY_SIZE - 1] = 1;
	global_context->order_sum[HISTORY_SIZE - 1] = 1;
	
	global_context->buffer = (ck_ring_t *)malloc(sizeof(ck_ring_t));
	void *data_buffer = malloc(4096);
	ck_ring_init(global_context->buffer, data_buffer, 4);
	
	dbg_threading("Structures allocated. (context=%p)\n",
	              global_context);
	
	pthread_barrier_wait(&start_barrier);
}