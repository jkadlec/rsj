#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include <sys/time.h>

#include <ck_ring.h>
#include "rb.h"

#include "debug.h"
#include "error.h"
#include "structures.h"
#include "helpers.h"
#include "table.h"
#include "init.h"
#include "worker.h"
#include "sync.h"
#include "iresultconsumer.h"

static uint64_t time_sum = 0;

//static const int affinity_map[15] = {16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23};

static const int affinity_map[15] = {1, 2, 3, 4, 5, 6, 7, 4, 20, 5, 21, 6, 22, 7, 23};

void dbg_print_order_indices()
{
dbg_threading_exec(
	dbg_threading("Order:\n");
	for (int i = 0; i < HISTORY_SIZE; i++) {
		dbg_threading("%d=%d ", i, global_context->order[i]);
	}
	dbg_threading("\n");
);
}

static struct timespec timespec_diff(struct timespec start, struct timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}


#define SIDE_BUYING 0
#define SIDE_SELLING 1

const double pi2 = 1.464225476445437;
const double atanconst = 0.25100268664303466;

inline double compute_fp_i(double equity, int pricebid_i)
{
	double f = 1.5 * (1 - equity - ((equity - 1) * (equity - 1)));
	double g = pi2 * atan(equity / 3.9) - (equity * atanconst);
	return (double)pricebid_i + f + g;
}

void result_testing(int seqNum, double fp_global_i, int index)
{
	unsigned int nano = 0;
#ifdef MEASURE_TIME
	struct timespec time;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
	struct timespec diff = timespec_diff(times[index], time);
	nano = diff.tv_nsec;
	time_sum += nano;
#endif
//	fprintf(stderr, "Result called: %d, %.9f (%d)\n", seqNum, fp_global_i,
//	        nano);
	if (seqNum == 1001) {
		destructor_c_style();
	}
}

inline void insert_data(rsj_data_in_t *data, int id)
{
				/* Insert this update to lookup structure. */
	if (data->side == SIDE_BUYING) {
		bids_insert(&(global_context->bids[data->instrument]),
		            data->order_index, data->specific_index,
		            data->volume, data->price);
		dbg_calc("Thread=%d seqNum=%d Instrument=%d index=%d Loading other data From history=%d\n",
		         id, data->seqNum, data->instrument,
		         data->specific_index,
		         prev_index_spec(data->specific_index));
		global_context->asks[data->instrument].history[data->specific_index] =
			global_context->asks[data->instrument].history[prev_index_spec(data->specific_index)];
	} else {
		asks_insert(&(global_context->asks[data->instrument]),
		            data->order_index, data->specific_index,
		            data->volume, data->price);
		dbg_calc("Thread=%d seqNum=%d Instrument=%d index=%d Loading other data From history=%d\n",
		         id, data->seqNum, data->instrument,
		         data->specific_index,
		         prev_index_spec(data->specific_index));
		global_context->bids[data->instrument].history[data->specific_index].price_bid = global_context->bids[data->instrument].history[prev_index_spec(data->specific_index)].price_bid;
		global_context->bids[data->instrument].history[data->specific_index].vol_bid = global_context->bids[data->instrument].history[prev_index_spec(data->specific_index)].vol_bid;
	}
	
	dbg_threading("Thread=%d: Data inserted.\n",
	              id);
	dbg_calc("Thread=%d seqNum=%d Instrument=%d Loading data From history=%d\n",
	         id, data->seqNum, data->instrument, data->specific_index);
}

inline void compute_return_fp_i(rsj_data_in_t *data, int id, int volbid,
                         int volask, int pricebid)
{
        /* Compute fp_i. */
        double fp_i = compute_fp_i(((double) volbid) / ((double)(volbid + volask)),
                                   pricebid);
        
        dbg_calc("Thread=%d: seqNum=%d fp_i=%f for instrument=%d (from data %d %d %d).\n",
                 id, data->seqNum, fp_i, data->instrument, pricebid,
	         volbid, volask);
        
        /* Wait until previous threads store their sums. */
        dbg_threading("Thread=%d order=%d seqNum=%d data->instrument=%d Waiting for index=%d to compute sum.\n",
                      id, data->order_index, data->seqNum, data->instrument,
                      prev_index(data->order_index));
        wait_until_set_order_sum(prev_index(data->order_index));
        sync_unset_order_sum(data->order_index);
        assert(global_context->order_sum[data->order_index] == 0);
        dbg_threading("Thread=%d order=%d data->seqNum=%d data->instrument=%d index=%d unlocked sum.\n",
                      id, data->order_index, data->seqNum, data->instrument, data->order_index - 1);
        double sum = global_context->sum_history[prev_index(data->order_index)];
                
        dbg_calc("Thread %d: data->data->seqNum=%d data->instrument=%d Index=%d Loading sum from history=%d =%f\n", id ,data->seqNum, data->instrument, data->order_index, (data->order_index - 1) % HISTORY_SIZE,
                 sum);
        
        /* Change fp_sum accordingly. */
        sum -= global_context->fp_i_history[data->instrument][prev_index_spec(data->specific_index)];
        sum += fp_i;
        double res = fp_i / sum;
        if (unlikely(isinf(res))) {
            res = 1.0;
        }
        dbg_calc("Thread %d: data->seqNum=%d data->instrument=%d: %f/%f\n",
               id, data->seqNum, data->instrument, fp_i, sum);
//	consumer.Result(data->seqNum, res);
        result_testing(data->seqNum, res, data->order_index);
        dbg_calc("%d: data->seqNum=%d History=%d Deducting from sum=%f\n", id, data->seqNum, data->specific_index, global_context->fp_i_history[data->instrument][(data->specific_index -1) % HISTORY_SIZE]);
        global_context->sum_history[data->order_index] = sum;
        dbg_calc("%d: data->seqNum=%d History=%d Adding to sum=%f\n",
               id, data->seqNum, data->order_index, fp_i);
        global_context->sum_history[data->order_index] = sum;
        dbg_calc("%d: data->seqNum=%d History=%d Saving sum=%f\n",
               id, data->seqNum, data->order_index,
               sum);
        dbg_calc("%d: data->seqNum=%d History=%d Saving fp_i=%f\n",
        id, data->seqNum, data->specific_index, fp_i);
        global_context->fp_i_history[data->instrument][data->specific_index] = fp_i;
        dbg_threading("%d: data->seqNum=%d index=%d Unlocking sums.\n",
        id, data->seqNum, data->order_index);
        sync_set_order_sum(data->order_index);
        /* Work done. */
}

inline void history_shift(rsj_data_in_t *data, int id)
{
        dbg_calc("Thread=%d seqNum=%d index=%d. Waiting for history unlock by index=%d.\n",
                 id, data->seqNum, data->order_index, (data->order_index - 1) % HISTORY_SIZE);
        dbg_print_order_indices();
        wait_until_set_order_sum(prev_index(data->order_index));
        sync_unset_order_sum(data->order_index);
        global_context->fp_i_history[data->instrument][data->specific_index] = 0.0;
        global_context->sum_history[data->order_index] =
            global_context->sum_history[prev_index(data->order_index)] - global_context->fp_i_history[data->instrument][prev_index_spec(data->specific_index)];
        dbg_calc("Thread=%d seqNum=%d Instrument=%d Index=%d reusing sum history (from history=%d sum=%f (deducted=%f)).\n",
                 id, data->seqNum, data->instrument, data->order_index,
	         data->order_index - 1,
	         global_context->sum_history[data->order_index],
               global_context->fp_i_history[data->instrument][prev_index_spec(data->specific_index)]);
        sync_set_order_sum(data->order_index);
}

void *worker_start(void *null_data)
{
	/* Obtain id. */
	dbg_threading("Thread started.\n");
	int id = __sync_fetch_and_add(&(global_context->global_id), 1);
	dbg_threading("Thread aquired ID=%d\n", id);
	/* Set affinity. */
	stick_this_thread_to_core(affinity_map[id]);
	/* Sync with other worker threads. */
	rsj_data_in_t *local_data = NULL;
	pthread_barrier_wait(&start_barrier);
	dbg_threading("Thread ID=%d past the barrier.\n", id);
	/* While computation not done */
	while (global_context->running) {
		/* Wait for work. */
		while(!ck_ring_dequeue_spmc(global_context->buffer,
		                           (void *)(&local_data))) {
			;
		}
		dbg_ring("dequeded seqNum=%d\n",
		         global_context->worker_data[id]->seqNum);
		
		sync_unset_order(next_index(local_data->order_index));
		sync_unset_order_sum(next_index(local_data->order_index));
			
		insert_data(local_data, id);
			
dbg_calc_exec(
		dbg_calc("seqNum=%d history=%d Sums: ",
		       local_data->seqNum, local_data->order_index);
		for (int i = 0; i < HISTORY_SIZE; i++) {
			dbg_calc("%d=%f, ", i, global_context->sum_history[i]);
		}
		dbg_calc("\n");
);
		
			/* Get the best buying price. */
			int pricebid = global_context->bids[local_data->instrument].history[local_data->specific_index].price_bid;
			/* Get the best price volume. */
			int volbid = global_context->bids[local_data->instrument].history[local_data->specific_index].vol_bid;
			/* Get the best ask volume. */
			int volask = global_context->asks[local_data->instrument].history[local_data->specific_index];
			if (unlikely(volask == 0 || volbid == 0 || pricebid == 0)) {
				/* Sum history unchanged. */
//				consumer.Result(seqNum, 0.0);
				result_testing(local_data->seqNum, 0.0, local_data->order_index);
        			dbg_calc("Thread=%d returning 0 for seqNum=%d - not enough data (%d %d %d).\n",
		                         id, local_data->seqNum, pricebid, volbid, volask);
				history_shift(local_data, id);
				continue;
			}
			
			compute_return_fp_i(local_data, id, volbid, volask,
			                    pricebid);
			
	}
	
	dbg_threading("Thread=%d DONE.\n", id);
	return NULL;
}

long timeval_diff(struct timeval *starttime, struct timeval *finishtime)
{
	long msec;
	msec=(finishtime->tv_sec-starttime->tv_sec)*1000;
	msec+=(finishtime->tv_usec-starttime->tv_usec)/1000;
	return msec;
}

void destructor_c_style()
{

#ifdef MEASURE_TIME
	struct timeval time;
	gettimeofday(&time, NULL);
	long diff = timeval_diff(&start_time, &time);
	printf("Total=%lu Mean=%llu\n", diff, time_sum/loaded);
#endif
	for (int i = 0; i < THREAD_COUNT; i++) {
		pthread_kill(global_context->worker_threads[i], 2);
	}
	global_context->running = 0;
}



