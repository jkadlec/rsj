#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "debug.h"
#include "error.h"
#include "tester.h"
#include "structures.h"

#define THREAD_COUNT 2;

#define spinlock_init pthread_spin_init
#define spinlock_lock pthread_spin_lock
#define spinlock_unlock pthread_spin_unlock

int tester_load_csv_file(const char *filename, rsj_data_t ***data,
                         size_t *data_count)
{
    	FILE *f = fopen(filename, "r");
	assert(f);
	
	dbg_test("Loading file=%s\n",
	         filename);
	
	*data = malloc(1024);
	assert(data);
	
	/* Read line by line. */
	rsj_data_t tmp_data;
	int ret = 0;
	size_t i = 0;
	while ((ret = fscanf(f, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f\n",
	                     &tmp_data.seqNum,
	                     &tmp_data.instrument,
	                     &tmp_data.price,
	                     &tmp_data.volume,
	                     &tmp_data.side,
	                     &tmp_data.priceBid_i,
	                     &tmp_data.volBid_i,
	                     &tmp_data.priceAsk_i,
	                     &tmp_data.volAsk_i,
	                     &tmp_data.equity_i,
	                     &tmp_data.f_i,
	                     &tmp_data.g_i,
	                     &tmp_data.fp_i,
	                     &tmp_data.fp_global_i)) == 14) {
		(*data)[i] = malloc(sizeof(rsj_data_t));
		assert((*data)[i]);
		(*data)[i]->seqNum = tmp_data.seqNum;
		(*data)[i]->instrument = tmp_data.instrument;
		(*data)[i]->price = tmp_data.price;
		(*data)[i]->volume = tmp_data.volume;
		(*data)[i]->side = tmp_data.side;
		(*data)[i]->volBid_i = tmp_data.volBid_i;
		(*data)[i]->volAsk_i = tmp_data.volAsk_i;
		(*data)[i]->priceAsk_i = tmp_data.priceAsk_i;
		(*data)[i]->priceBid_i = tmp_data.priceBid_i;
		(*data)[i]->equity_i = tmp_data.equity_i;
		(*data)[i]->f_i = tmp_data.f_i;
		(*data)[i]->fp_i = tmp_data.fp_i;
		(*data)[i]->g_i = tmp_data.g_i;
		(*data)[i]->fp_global_i = tmp_data.fp_global_i;
		i++;
	}
	
	*data_count = ++i;
	dbg_test("Loaded %d queries.\n",
	         i);
	return RSJ_EOK;
}

int tester_start_computation(rsj_data_t **data,
                             size_t data_count,
                             void *params)
{
	context_t context;
	Initialize(&context);
	for (size_t i = 0; i < data_count; i++) {
		Update(&context,
		       data[i]->seqNum, data[i]->instrument, data[i]->price,
		       data[i]->volume, data[i]->side);
	}
	
	for (size_t i = 0; i < data_count; i++) {
		//collect all the results
	}
}

void consumer_thread_func(void *data)
{
	context_t *context = (context_t *)data;
}

void scheduler_thread_func(void *data)
{
	context_t *context = (context_t *)data;
	/* Main cycle. */
	while (!context->end && context->data_count) {
		
	}
}

static char global_id = 0;

struct worker_context {
	int id = 0;
	int working = 0;
};

typedef struct worker_context worker_context_t;



void worker_start(void *data)
{
	context_t *global_context = (context_t *)data;
	/* Create private thread context. */
	worker_context_t thread_context;
	/* TODO atomic. */
	/* Obtain id. */
	int id = __sync_fetch_and_add(&global_context->global_id, 1);
	dbg_threading("Thread aquired ID=%d\n", id);
	/* Sync with other worker threads. */
	pthread_barrier_wait(global_context->global_worker_sync);
	/* While computation not done */
	while (global_context->running) {
		/* Wait for work. */
		if (global_context->worker[i].go) {
			/* Get work. */
			/* Do work. Only compute fp_i, hold its spinlock while*/
			spinlock_lock(global_context->
			double_fp_i = compute_fp_i();
			
			/* Work done. */
		}
	
		/* Call worker function. */
	
		/* Tell scheduler that this thread can work again. */
		global_context->[];
	}
}

//TODO move to c++ class once done
void Initialize(context_t *context)
{
	context->data_count = 0;
	/* Init spinlocks. */
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
		spinlock_init(&(context->fp_i_locks), PTHREAD_PROCESS_SHARED);
	}
	
	pthread_barrier_init(context->global_worker_sync, NULL, THREAD_COUNT);
	
	/* Start scheduler thread. */
	//might not be needed
//	pthread_create(context->scheduler_thread, NULL, scheduler_thread_func,
//	               context);
	/* Start worker threads. */
	pthread_create();
}

void Update(context_t *context,
            int seqNum, int instrument, int price, int volume, int side)
{
	/* Store incoming data to buffer. */
	size_t index = context->data_count;
	context->in_array[index].instrument = instrument;
	context->in_array[index].price = price;
	context->in_array[index].volume = volume;
	context->in_array[index].side = side;
	/* Scheduler can consume this update. */
//	context->data_count++;
	/* */
}

void Result(context_t *context,
            int seqNum, double fp_global_i)
{
}
