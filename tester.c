#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <limits.h>

#include "debug.h"
#include "error.h"
#include "tester.h"
#include "structures.h"

pthread_barrier_t tmp_barrier;

#define unlikely(x) __builtin_expect(!!(x), 0)

#define spinlock_init pthread_spin_init
#define spinlock_lock pthread_spin_lock
#define spinlock_unlock pthread_spin_unlock

void dbg_print_order_indices(const context_t *my_context)
{
dbg_threading_exec(
	dbg_threading("Order:\n");
	for (int i = 0; i < HISTORY_SIZE; i++) {
		dbg_threading("%d=%d ", i, my_context->order[i]);
	}
	dbg_threading("\n");
);
}

static void bids_insert(context_t *global_context,
                        lookup_bid_t *bids,
                        spinlock_t *instrument_lock,
                        int history_index, int specific_index,
                        int vol_bid, int price_bid)
{
	dbg_calc("Thread with history index=%d: Inserting bid, volBid=%d,"
	         "priceBid=%d\n",
	         specific_index, vol_bid, price_bid);
	/* Wait until previous data is inserted. */
	unsigned char index = (unsigned char)(history_index - 1) % HISTORY_SIZE;
dbg_threading_exec(
	dbg_threading("Order:\n");
	for (int i = 0; i < HISTORY_SIZE; i++) {
		dbg_threading("%d=%d ", i, global_context->order[i]);
	}
	dbg_threading("\n");
);
	dbg_calc("%d:Waiting for thread with order=%d to finish with adding.\n",
	         specific_index, index);
	while (__sync_bool_compare_and_swap(&(global_context->order[index]), 0, 0)) {
		;
	}
	__sync_lock_test_and_set(&(global_context->order[history_index]), 0);
	dbg_calc("%d:All good, free to go.\n", specific_index);
	bids->data[price_bid] = vol_bid;
	if (unlikely(vol_bid == 0)) {
		dbg_calc("Thread with history index=%d: Removing price.\n",
			specific_index);
		if (price_bid == bids->price_bid) {
			dbg_calc("Thread with history index=%d: Was best price.\n",
				specific_index);
			/* TODO binary nebo tak neco. */
			size_t index = price_bid;
			for (size_t i = price_bid - 1;
			 i != 0 && index == price_bid;
			 i--) {
				 if (bids->data[i] != 0) {
				    index = i;
				}
			}
			if (unlikely(index == price_bid)) {
				dbg_calc("No better price.\n");
				bids->price_bid = 0;
				bids->vol_bid = 0;
				bids->history[specific_index].price_bid = 0;
				bids->history[specific_index].vol_bid = 0;
			} else {
				dbg_calc("Thread with history index=%d: New best price=%d,"
				         " volume=%d. (used to be %d)\n",
				         specific_index, index, bids->data[index],
				         price_bid);
				bids->price_bid = index;
				bids->vol_bid = bids->data[index];
				bids->history[specific_index].price_bid = index;
				bids->history[specific_index].vol_bid = bids->data[index];
			}
		} else {
			bids->history[specific_index].price_bid = bids->history[(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE].price_bid;
			bids->history[specific_index].vol_bid = bids->history[(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE].vol_bid;
		}
	} else if (price_bid >= bids->price_bid) {
		dbg_calc("Thread with history index=%d: new best price=%d (bid).\n",
		         specific_index, price_bid);
		bids->price_bid = price_bid;
		bids->vol_bid = vol_bid;
		bids->history[specific_index].price_bid = bids->price_bid;
		bids->history[specific_index].vol_bid = bids->vol_bid;
	} else {
		dbg_calc("Thread with history index=%d: No change, using old values (bid).\n",
		         specific_index);
		/* Nothing changed - we need to use the last value. */
		bids->history[specific_index].price_bid = bids->history[(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE].price_bid;
		bids->history[specific_index].vol_bid = bids->history[(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE].vol_bid;
	}
	
	/* Tell other threads that this thread is done with insertion. */
	__sync_lock_test_and_set(&(global_context->order[history_index]), 1);
	dbg_threading("Order=%d unlocked.\n", history_index);
	dbg_calc("Thread with history index=%d: Price after insert=%d, volume=%d\n",
	         history_index,
	         bids->history[specific_index].price_bid,
	         bids->history[specific_index].vol_bid);
}

void asks_insert(context_t *global_context,
                 lookup_ask_t *asks,
                 spinlock_t *instrument_lock,
                 int history_index, int specific_index,
                 int vol_ask, int price_ask)
{
	dbg_calc("Thread with history index=%d: Inserting ask, volAsk=%d,"
	         "priceAsk=%d (current best = %d %d)\n",
	          specific_index, vol_ask, price_ask, asks->price_ask, 
	         asks->vol_ask);
	dbg_print_order_indices(global_context);
	unsigned char index = (unsigned char)(history_index - 1) % HISTORY_SIZE;
	dbg_calc("%d:Waiting for thread with order=%d to finish with adding.\n",
			history_index, index);
	while (__sync_bool_compare_and_swap(&(global_context->order[index]), 0, 0)) {
		;
	}
	__sync_lock_test_and_set(&global_context->order[history_index], 0);
	dbg_calc("%d:All good, free to go.\n", specific_index);
	asks->data[price_ask] = vol_ask;
	if (unlikely(vol_ask == 0)) {
		dbg_calc("Thread with history index=%d: Removing price.\n",
		         specific_index);
		if (price_ask == asks->price_ask) {
			dbg_calc("Thread with history index=%d: Was best price.\n",
			         specific_index);
			/* TODO binary nebo tak neco. */
			size_t index = price_ask;
			for (size_t i = price_ask + 1;
			     i < 100 && index == price_ask;
			     i++) {
				if (asks->data[i] != 0) {
					index = i;
				}
			}
			if (unlikely(index == price_ask)) {
				dbg_calc("No better price.\n");
				asks->price_ask = INT_MAX;
				asks->vol_ask = 0;
				asks->history[specific_index] = 0;
			} else {
				dbg_calc("Thread with history index=%d: New best price=%d, volume=%d.\n",
				         specific_index, index, asks->data[index]);
				asks->price_ask = index;
				asks->vol_ask = asks->data[index];
				asks->history[specific_index] = asks->data[index];
			}
		} else {
			asks->history[specific_index] =
				asks->history[(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE];
		}
	} else if (price_ask <= asks->price_ask) {
		dbg_calc("Thread with history index=%d: new best price (ask).\n",
		         specific_index);
		asks->price_ask = price_ask;
		asks->vol_ask = vol_ask;
		asks->history[specific_index] = vol_ask;
	} else {
		dbg_calc("%d: no change, using history.\n", specific_index);
		asks->history[specific_index] = asks->history[(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE];
	}
	
	/* Tell other threads that this thread is done with insertion. */
	__sync_lock_test_and_set(&(global_context->order[history_index]), 1);
	dbg_threading("Order=%d unlocked.\n", history_index);
	dbg_calc("Thread with history index=%d: Price after insert = %d, volask=%d\n",
	         specific_index, asks->price_ask,
	         asks->history[specific_index]);

}

#define SIDE_BUYING 0
#define SIDE_SELLING 1
#define PRICE_CANCEL 0

typedef struct worker_context worker_context_t;

const double pi2 = 1.464225476445437;
const double atanconst = 0.25100268664303466;

double compute_fp_i(double equity, int pricebid_i)
{
	double f = 1.5 * (1 - equity - ((equity - 1) * (equity - 1)));
	double g = pi2 * atan(equity / 3.9) - (equity * atanconst);
	return (double)pricebid_i + f + g;
}

int shared = 0;

void *worker_start(void *data)
{
	context_t *global_context = (context_t *)data;
	/* Obtain id. */
	int id = __sync_fetch_and_add(&global_context->global_id, 1);
	dbg_threading("Thread aquired ID=%d\n", id);
	/* Sync with other worker threads. */
	pthread_barrier_wait(&tmp_barrier);
	dbg_threading("Thread ID=%d past the barrier.\n", id);
	/* While computation not done */
	while (global_context->running) {
		/* Wait for work. */
		if (__sync_bool_compare_and_swap(&(global_context->worker_data[id].go), 1, 1)) {
			dbg_threading("Thread=%d starting computation with seqNum=%d.\n",
			              id, global_context->worker_data[id].seqNum);
			/* Get work. */
			int instrument =
				global_context->worker_data[id].instrument;
			int seqNum =
				global_context->worker_data[id].seqNum;
			int price =
				global_context->worker_data[id].price;
			int side =
				global_context->worker_data[id].side;
			int volume =
				global_context->worker_data[id].volume;
			int order_index =
				global_context->worker_data[id].order_index;
			int specific_index =
				global_context->worker_data[id].specific_index;
			assert(order_index < HISTORY_SIZE);
			assert(specific_index < SPECIFIC_HISTORY_SIZE);
//			sequence testing
//			printf("seqNum=%d index=%d, waiting for previous calc to be completed=%d\n",
//			       seqNum,
//			       order_index, (unsigned char)(order_index - 1) % HISTORY_SIZE);
//			while (__sync_bool_compare_and_swap(&(global_context->order[(unsigned char)(order_index - 1) % HISTORY_SIZE]), 0, 0)) {
//				;
//			}
//			__sync_lock_test_and_set(&(global_context->order[order_index]), 0);
//			shared++;
//			printf("seqNum=%d index=%d, it has modified the shared variable to be=%d\n",
//			       seqNum,
//			       order_index, shared);
//			printf("seqNum=%d index=%d Order before: ",seqNum, order_index);
//			for (int i = 0; i < HISTORY_SIZE; i++) {
//				printf("%d", global_context->order[i]);
//			}		
//			printf("\n");
//			printf("seqNum=%d index=%d Order after : ",seqNum, order_index);
//			for (int i = 0; i < HISTORY_SIZE; i++) {
//				if (i == order_index) {
//					printf("1");
//				} else {
//					printf("%d", global_context->order[i]);
//				}
//			}
//			printf("\n");
//			assert(seqNum == shared);
//			__sync_lock_test_and_set(&(global_context->order[order_index]), 1);
//			__sync_lock_test_and_set(&global_context->worker_data[id].go,
//			                         0);
//			continue;
dbg_calc_exec(
		dbg_calc("seqNum=%d history=%d Sums: ",
		       seqNum, order_index);
		for (int i = 0; i < HISTORY_SIZE; i++) {
			dbg_calc("%d=%f, ", i, global_context->sum_history[i]);
		}
		dbg_calc("\n");
);
			/* Insert this update to lookup structure. */
			if (side == SIDE_BUYING) {
				bids_insert(global_context,
				            &(global_context->bids[instrument]),
				            NULL, order_index, specific_index, volume, price);
				dbg_calc("Thread=%d seqNum=%d Instrument=%d index=%d Loading other data From history=%d\n",
				         id, seqNum, instrument, specific_index, (unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE);
				global_context->asks[instrument].history[specific_index] = global_context->asks[instrument].history[(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE];
			} else {
				asks_insert(global_context,
				            &(global_context->asks[instrument]),
				            NULL, order_index, specific_index, volume, price);
				dbg_calc("Thread=%d seqNum=%d Instrument=%d index=%d Loading other data From history=%d\n",
				         id, seqNum, instrument, specific_index, (unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE);
				global_context->bids[instrument].history[specific_index].price_bid = global_context->bids[instrument].history[(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE].price_bid;
				global_context->bids[instrument].history[specific_index].vol_bid = global_context->bids[instrument].history[(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE].vol_bid;
			}
			
			dbg_threading("Thread=%d: Data inserted.\n",
			              id);
			dbg_calc("Thread=%d seqNum=%d Instrument=%d Loading data From history=%d\n",
			         id, seqNum, instrument, specific_index);
			/* Get the best buying price. */
			int pricebid = global_context->bids[instrument].history[specific_index].price_bid;
			/* Get the best price volume. */
			int volbid = global_context->bids[instrument].history[specific_index].vol_bid;
			/* Get the best ask volume. */
			int volask = global_context->asks[instrument].history[specific_index];
			if (unlikely(volask == 0 || volbid == 0 || pricebid == 0)) {
				/* Sum history unchanged. */
				Result(global_context, seqNum, 0.0);
				dbg_calc("Thread=%d returning 0 for seqNum=%d - not enough data (%d %d %d).\n",
				         id, seqNum, pricebid, volbid, volask);
				dbg_calc("Thread=%d seqNum=%d index=%d. Waiting for history unlock by index=%d.\n",
				         id, seqNum, order_index, (unsigned char)(order_index - 1) % HISTORY_SIZE);
				dbg_print_order_indices(global_context);
				while (__sync_bool_compare_and_swap(&global_context->order_sum[(unsigned char)(order_index - 1) % HISTORY_SIZE], 0, 0)) {
					;
				}
				__sync_lock_test_and_set(&(global_context->order_sum[order_index]), 0);
				shared++;
				if (shared != seqNum) {
					dbg_threading("%d %d\n", shared, seqNum);
					assert(0);
				}
				global_context->fp_i_history[instrument][specific_index] = 0.0;
				global_context->sum_history[order_index] =
					global_context->sum_history[(unsigned char)(order_index - 1) % HISTORY_SIZE] - global_context->fp_i_history[instrument][(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE];
				dbg_calc("Thread=%d seqNum=%d Instrument=%d Index=%d reusing sum history (from history=%d sum=%f (deducted=%f)).\n",
				         id, seqNum, instrument, order_index, order_index - 1, global_context->sum_history[order_index],
				       global_context->fp_i_history[instrument][(unsigned char)(specific_index - 1) % SPECIFIC_HISTORY_SIZE]);
				__sync_lock_test_and_set(&global_context->order_sum[order_index], 1);
				__sync_lock_test_and_set(&global_context->worker_data[id].go,
				                         0);
				continue;
			}
			
			/* Do work. Only compute fp_i. */
			double fp_i = compute_fp_i(((double) volbid) / ((double)(volbid + volask)),
			                           pricebid);
			
			dbg_calc("Thread=%d: seqNum=%d fp_i=%f for instrument=%d (from data %d %d %d).\n",
			         id, seqNum, fp_i, instrument, pricebid, volbid, volask);
			
			/* Wait until previous threads store their sums. */
			dbg_threading("Thread=%d order=%d seqNum=%d instrument=%d Waiting for index=%d to compute sum.\n",
			              id, order_index, seqNum, instrument,
			              (unsigned char)(order_index - 1) % HISTORY_SIZE);
			while (__sync_bool_compare_and_swap(&global_context->order_sum[(unsigned char)(order_index - 1) % HISTORY_SIZE], 0, 0)) {
			}
			__sync_lock_test_and_set(&(global_context->order_sum[order_index]), 0);
			shared++;
			if (shared != seqNum) {
				dbg_threading("%d %d\n", shared, seqNum);
				assert(0);
			}
			assert(global_context->order_sum[order_index] == 0);
			dbg_threading("Thread=%d order=%d seqNum=%d instrument=%d index=%d unlocked sum.\n",
			              id, order_index, seqNum, instrument, order_index - 1);
			double sum = global_context->sum_history[(unsigned char)(order_index - 1) % HISTORY_SIZE];
					
			dbg_calc("Thread %d: seqNum=%d Instrument=%d Index=%d Loading sum from history=%d =%f\n", id ,seqNum, instrument, order_index, (unsigned char)(order_index - 1) % HISTORY_SIZE,
			         sum);
			
			/* Change fp_sum accordingly. */
			sum -= global_context->fp_i_history[instrument][(unsigned char)(specific_index -1) % HISTORY_SIZE];
			sum += fp_i;
			double res = fp_i / sum;
			if (unlikely(isinf(res))) {
				res = 1.0;
			}
			dbg_calc("Thread %d: seqNum=%d Instrument=%d: %f/%f\n",
			       id, seqNum, instrument, fp_i, sum);
			Result(global_context, seqNum, res);//compute_fp_global(global_context->fp_sums,
			dbg_calc("%d: seqNum=%d History=%d Deducting from sum=%f\n", id, seqNum, specific_index, global_context->fp_i_history[instrument][(unsigned char)(specific_index -1) % HISTORY_SIZE]);
			global_context->sum_history[order_index] = sum;
			dbg_calc("%d: seqNum=%d History=%d Adding to sum=%f\n",
			       id, seqNum, order_index, fp_i);
			global_context->sum_history[order_index] = sum;
			dbg_calc("%d: seqNum=%d History=%d Saving sum=%f\n",
			       id, seqNum, order_index,
			       sum);
			dbg_calc("%d: seqNum=%d History=%d Saving fp_i=%f\n",
			id, seqNum, specific_index, fp_i);
			global_context->fp_i_history[instrument][specific_index] = fp_i;
			dbg_threading("%d: seqNum=%d index=%d Unlocking sums.\n",
			id, seqNum, order_index);
			__sync_lock_test_and_set(&global_context->order_sum[order_index], 1);
			/* Work done. */
			__sync_lock_test_and_set(&global_context->worker_data[id].go,
			                         0);
		}
	}
	
	dbg_threading("Thread=%d DONE.\n", id);
	return NULL;
}

spinlock_t file_lock;

//TODO move to c++ class once done
void Initialize(context_t *my_context)
{
	/* Init basic variables. */
	my_context->global_id = 0;
	my_context->running = 1;
	my_context->order_index = 0;
	int ret = pthread_barrier_init(&tmp_barrier, NULL,
	                               THREAD_COUNT + 1);
	assert(ret == 0);
	spinlock_init(&tmp_lock, PTHREAD_PROCESS_SHARED);
	
	/* Init worker structures. */
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
		my_context->worker_data[i].go = 0;
		my_context->worker_data[i].instrument = 0;
		my_context->worker_data[i].order_index = 0;
		my_context->worker_data[i].price = 0;
		my_context->worker_data[i].seqNum = 0;
		my_context->worker_data[i].side = 0;
		my_context->worker_data[i].specific_index = 0;
		my_context->worker_data[i].volume = 0;
	}
	
	/* Init spinlocks. */
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
//		spinlock_init(&(my_context->insert_instrument_lock[i]),
//		              PTHREAD_PROCESS_SHARED);
		my_context->fp_i_history[i] = malloc(HISTORY_SIZE * sizeof(double));
		for (int j = 0; j < HISTORY_SIZE; j++) {
			my_context->fp_i_history[i][j] = 0.0;
		}
		my_context->order_indices_bid[i] = 0;
		assert(my_context->fp_i_history[i]);
	}
	
	/* Start worker threads. */
	for (int i = 0; i < THREAD_COUNT; i++) {
		pthread_create(&(my_context->worker_threads[i]),
		               NULL, worker_start, my_context);
	}
	
	dbg_threading("Threads started.\n");
	
	/* Init lookup structures. */
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
		//toto bude fungovat, ale bidy potrebujou vic dat nez asky
		my_context->asks[i].data =
			malloc(200 * sizeof(int));
		assert(my_context->asks[i].data);
		for (int j = 0; j < 200 ; j++) {
			my_context->asks[i].data[j] = 0;
		}
		my_context->asks[i].vol_ask = 0;
		my_context->asks[i].price_ask = INT_MAX;
		my_context->bids[i].data =
			malloc(200 * sizeof(int));
		assert(my_context->bids[i].data);
                for (int j = 0; j < 200; j++) {
                    my_context->bids[i].data[j] = 0;
                }
		for (int j = 0; j < HISTORY_SIZE; j++) {
			my_context->bids[i].history[j].price_bid = 0;
			my_context->bids[i].history[j].vol_bid = 0;
			my_context->asks[i].history[j] = 0;
		}
		my_context->bids[i].price_bid = 0;
		my_context->bids[i].vol_bid = 0;
	}
	
	for (int i = 0; i < HISTORY_SIZE; i++) {
		my_context->order[i] = 0;
		my_context->order_sum[i] = 0;
		my_context->sum_history[i] = 0.0;
	}
	
	my_context->order[HISTORY_SIZE - 1] = 1;
	//todo je potreba order_sum? nestaci order? jsou tam jine informace?
	my_context->order_sum[HISTORY_SIZE - 1] = 1;
	my_context->order[128] = 1;
	
	dbg_threading("Structures allocated. (context=%p)\n",
	              my_context);
	
	pthread_barrier_wait(&tmp_barrier);
}

int pool_return_free_thread_id(rsj_data_in_t *worker_data)
{
	while (1) {
		for (int i = 0; i < THREAD_COUNT; i++) {
			if (worker_data[i].go == 0) {
				return i;
			}
		}
	}
	
	return -1;
}

void Update(context_t *my_context, int seqNum, int instrument,
            int price, int volume, int side)
{
	char index = (unsigned char)(my_context->order_index) % HISTORY_SIZE;
	/* Find available thread to give work to. Blocks if all threads are full, at least for now. */
//	dbg_calc("Getting free thread ID.\n");
	int id = -1;
	while (id < 0) {
		for (int i = 0; i < THREAD_COUNT; i++) {
			if (__sync_bool_compare_and_swap(&my_context->worker_data[i].go,
			                                0, 0)) {
				id = i;
			}
		}
	}
	
	/* Fill thread's data. Computation will start automatically. */
	my_context->worker_data[id].seqNum = seqNum;
	my_context->worker_data[id].instrument = instrument;
	my_context->worker_data[id].side = side;
	my_context->worker_data[id].volume = volume;
	my_context->worker_data[id].price = price;
	my_context->worker_data[id].order_index = (unsigned char)(my_context->order_index++) % HISTORY_SIZE;
	//todo bitove pole
	my_context->worker_data[id].specific_index = (unsigned char)(my_context->order_indices_bid[instrument]++) % SPECIFIC_HISTORY_SIZE;
	my_context->order[(unsigned char)(my_context->worker_data[id].order_index + 1) % HISTORY_SIZE] = 0;
//	assert(my_context->order_sum[(unsigned char)(my_context->worker_data[id].order_index + 1) % HISTORY_SIZE] == 1);
	my_context->order_sum[(unsigned char)(my_context->worker_data[id].order_index + 1) % HISTORY_SIZE] = 0;
	__sync_lock_test_and_set(&my_context->worker_data[id].go,
	                         1);
//	fprintf(stderr, "Update ID=%d called.\n",
//	         seqNum);
}


void Result(context_t *context,
            int seqNum, double fp_global_i)
{
	fprintf(stderr, "Result called: %d, %f\n", seqNum, fp_global_i);
	if (seqNum == 1000) {
		context->running = 0;
	}
}

void Destructor(context_t *context)
{
	context->running = 0;
}
