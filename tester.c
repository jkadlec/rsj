#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "debug.h"
#include "error.h"
#include "tester.h"
#include "structures.h"

pthread_barrier_t tmp_barrier;

#define AVAILABLE_MEMORY 2 * 1024 * 100

#define unlikely(x) __builtin_expect(!!(x), 0)

//instrumenty se muzou vkladat do struktury paralelne, ale vlozeni pro instrumenty se musi serializovat - jak?
//pred vracenim fp_i_total musim vedet, ze vlakna predemnou uz dobehla a maji zapsano v fp_sum, stejne tak jsou potreba predchozi fp_i pro vsechna asi, ale to muze byt nejak ulozene pospolu
//pro kazdy seqnum potrebuju:
//ceny budou stacit jedny globalni, ty se upravuji je pro stejne instrumenty, ale jako historie bude stacit ten nejlepsi a nejhorsi, to se porovna s tim, co tam vkladam ja a je to. Jen v pripade, ze to odstranuju, se bude muset neco dit a to bude muset mit sakra prioritu, jinak to bude pomale.
//				vlozit, ceny, volumy, fp_n

#define spinlock_init pthread_spin_init
#define spinlock_lock pthread_spin_lock
#define spinlock_unlock pthread_spin_unlock

void bids_insert(context_t *global_context,
                 lookup_bid_t *bids, int *history,
                 spinlock_t *instrument_lock,
                 int history_index, int vol_bid, int price_bid)
{
	dbg_calc("Thread with history index=%d: Inserting bid, volBid=%d,"
	         "priceBid=%d\n",
	         history_index, vol_bid, price_bid);
	/* Insert into array. */
	assert(bids->data[price_bid] == 0 || vol_bid == 0);
	//toto nebude fungovat, ale oh well.
	/* Wait until previous data is inserted. */
	// TODO musi byt nezavisle na bid a ask
	dbg_calc("%d:Waiting for other threads to finish with adding.\n",
	         history_index);
	while (!global_context->order[(history_index - 1) % 128]) {
		;
	}
	dbg_calc("%d:All good, free to go.\n", history_index);
	bids->data[price_bid] = vol_bid;
	if (unlikely(vol_bid == 0)) {
		dbg_calc("Thread with history index=%d: Removing price.\n",
			history_index);
		if (price_bid == bids->price_bid) {
			dbg_calc("Thread with history index=%d: Was best price.\n",
				history_index);
			/* TODO binary nebo tak neco. */
			size_t index = price_bid;
			for (size_t i = price_bid - 1;
			 i < 0 && index == price_bid;
			 i--) {
				 if (bids->data[i] != 0) {
				    index = i;
				}
			}
			if (unlikely(index == price_bid)) {
				dbg_calc("No better price.\n");
				bids->price_bid = 0;
				bids->vol_bid = 0;
				bids->history[history_index].price_bid = 0;
				bids->history[history_index].vol_bid = 0;
			} else {
				dbg_calc("Thread with history index=%d: New best price=%d, volume=%d.\n",
				         history_index, index, bids->data[index]);
				bids->price_bid = index;
				bids->vol_bid = bids->data[index];
				bids->history[history_index].price_bid = index;
				bids->history[history_index].vol_bid = bids->data[index];
			}
		}
	} else if (price_bid > bids->price_bid) {
		dbg_calc("Thread with history index=%d: new best price (bid).\n",
		         history_index);
		bids->price_bid = price_bid;
		bids->vol_bid = vol_bid;
		bids->history[history_index].price_bid = bids->price_bid;
		bids->history[history_index].vol_bid = bids->vol_bid;
	} else {
		/* Nothing changed - we need to use the last value. */
		bids->history[history_index].price_bid = bids->history[(history_index - 1) % 128].price_bid;
		bids->history[history_index].vol_bid = bids->history[(history_index - 1) % 128].vol_bid;
	}
	
	/* Tell other threads that this thread is done with insertion. */
	global_context->order[history_index] = 1;
	dbg_calc("Thread with history index=%d: Price after insert=%d, volume=%d\n",
	         history_index,
	         bids->history[history_index].price_bid,
	         bids->history[history_index].vol_bid);
}

void asks_insert(context_t *global_context,
                 lookup_ask_t *asks, rsj_double_t *history,
                 spinlock_t *instrument_lock,
                 int history_index, int vol_ask, int price_ask)
{
	dbg_calc("Thread with history index=%d: Inserting ask, volAsk=%d,"
	         "priceAsk=%d\n",
	          history_index, vol_ask, price_ask);
	/* Insert into array. */
	assert(asks->data[price_ask] == 0 || vol_ask == 0);
	//toto nebude fungovat, ale oh well.
//	spinlock_lock(instrument_lock);
	dbg_calc("%d:Waiting for other threads to finish with adding.\n",
	         history_index);
	while (!global_context->order[(history_index - 1) % 128]) {
		;
	}
	dbg_calc("%d:All good, free to go.\n", history_index);
	asks->data[price_ask] = vol_ask;
	//todo tady muze byt race
	if (unlikely(vol_ask == 0)) {
		dbg_calc("Thread with history index=%d: Removing price.\n",
		         history_index);
		if (price_ask == asks->price_ask) {
			dbg_calc("Thread with history index=%d: Was best price.\n",
			         history_index);
			/* TODO binary nebo tak neco. */
			size_t index = price_ask;
			for (size_t i = price_ask + 1;
			     i < 99999 && index == price_ask;
			     i++) {
				if (asks->data[i] != 0) {
					index = i;
				}
			}
                        if (unlikely(index == price_ask)) {
                            dbg_calc("No better price.\n");
                            asks->price_ask = 0;
                            asks->vol_ask = 0;
			asks->history[history_index] = 0;
                        } else {
				dbg_calc("Thread with history index=%d: New best price=%d, volume=%d.\n",
				         history_index, index, asks->data[index]);
				asks->price_ask = index;
				asks->vol_ask = asks->data[index];
				asks->history[history_index] = asks->data[index];
			}
		}
	} else if (price_ask < asks->price_ask) {
		dbg_calc("Thread with history index=%d: new best price (ask).\n",
		         history_index);
		asks->price_ask = price_ask;
		asks->vol_ask = vol_ask;
		asks->history[history_index] = vol_ask;
	} else {
		dbg_calc("%d: no change, using history.\n", history_index);
		asks->history[history_index] = asks->history[(history_index - 1) % 128];
	}
	
//	asks->history[history_index].price_bid = bids->price_bid;
//	spinlock_unlock(instrument_lock);
	/* Tell other threads that this thread is done with insertion. */
	global_context->order[history_index] = 1;
	dbg_calc("Thread with history index=%d: Price after insert, volask=%d\n",
	         history_index,
	         asks->history[history_index]);

}

int tester_load_csv_file(const char *filename, rsj_data_t ***data,
                         size_t *data_count)
{
	FILE *f = fopen(filename, "r");
	assert(f);
	
	dbg_test("Loading file=%s\n",
	         filename);
	
	*data = malloc(10240);
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

#define SIDE_BUYING 0
#define SIDE_SELLING 1
#define PRICE_CANCEL 0

typedef struct worker_context worker_context_t;

const double pi2 = 1.464225476445437;
const double atanconst = 1.5;

double compute_fp_i(double equity, int pricebid_i)
{
	double f = 1.5 * (1 - equity - ((equity - 1) * (equity - 1)));
	double g = pi2 * atan(equity / 3.9) - equity * atanconst;
	return pricebid_i + f + g;
}

//double do_work(int 
//               minmax_t *minmax,
//               int seqNum,
//               int instrument,
//               int price, int volume, int side,
//               double fp_i_prev,
//               double fp_sum)
//{
//	/* Lock fpi lock. */
//	double equity = (volbid + volask) * volbid;
////	spinlock_lock(&(context->fp_i_locks[instrument]));
//	/* Calculate fp_i */
//	double fpi = compute_fp_i(equity, pricebid);
//	/* Unlock fpi lock. */
////	spinlock_unlock(&(context->fp_i_locks[instrument]));
//	return fpi / (fp_sum - fp_i_prev + fpi);
//}

//void sums_add_fp_i(void *sums, int seqNum, double fp_i)
//{
//	double fp_i_previous = sums
//}

//inline double compute_fp_global(void *sums, int seqNum, double fp_i)
//{
//	return fp_i / sums_lookup_sum(sums, seqNum);
//}

void worker_start(void *data)
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
//		dbg_threading("Thread=%d waiting. (%d)\n",
//		              global_context->worker_data[id].go);
		/* Wait for work. */
		if (global_context->worker_data[id].go) {
			dbg_threading("Thread=%d starting computation.\n",
			              id);
			/* Get work. */
			rsj_data_in_t in_data;
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
			if (side == SIDE_BUYING) {
				/* Lock instrument lock. */
//				spinlock_lock(global_context->fp_i_locks[instrument]);
				/* Insert this update to lookup structure. */
				bids_insert(global_context,
				            &(global_context->bids[instrument]),
				            NULL, order_index, volume, price);
				global_context->asks[instrument].history[order_index % 128] = global_context->asks[instrument].history[(order_index - 1) % 128];
			} else {
//				spinlock_lock(global_context->fp_i_locks[instrument]);
				asks_insert(global_context,
				            &(global_context->asks[instrument]),
				            NULL, order_index, volume, price);
				global_context->bids[instrument].history[order_index % 128].price_bid = global_context->bids[instrument].history[(order_index - 1) % 128].price_bid;
				global_context->bids[instrument].history[order_index % 128].vol_bid = global_context->bids[instrument].history[(order_index - 1) % 128].vol_bid;
			}
			
			dbg_threading("Thread=%d: Data inserted.\n",
			              id);
			
			if (unlikely(price == PRICE_CANCEL)) {
				dbg_calc("Thread=%d: Instrument %d cancelled.\n",
				         id, instrument);
			}
			/* Get the best buying price. */
			int pricebid = global_context->bids[instrument].history[order_index].price_bid;
			/* Get the best price volume. */
			int volbid = global_context->bids[instrument].history[order_index].vol_bid;
			/* Get the best ask volume. */
			int volask = global_context->asks[instrument].history[order_index];
			if (unlikely(volask == 0.0 || volbid == 0.0)) {
				Result(global_context, seqNum, 0.0);
				dbg_calc("Thread=%d returning 0 for seqNum=%d - not enough data (%d %d %d).\n",
				         id, seqNum, pricebid, volbid, volask);
				global_context->worker_data[id].go = 0;
				continue;
			}
			
			/* Do work. Only compute fp_i. */
			double fp_i = compute_fp_i(volbid / (volbid + volask),
			                           pricebid);
			
			dbg_calc("Thread=%d: fp_i=%f for instrument=%d.\n",
			         id, fp_i, instrument);
			
			/* Change fp_sum accordingly. */
//			sums_add_fp_i(global_context->fp_sums, seqNum, fp_i);
			double sum = global_context->sum_history[(order_index - 1) % 128];
			sum -= global_context->fp_i_history[instrument][(order_index -1) % 128];
			sum += fp_i;
			
			Result(global_context, seqNum, fp_i / sum);//compute_fp_global(global_context->fp_sums,
//			                                 seqNum, fp_i));
			global_context->sum_history[order_index] = sum;
			global_context->fp_i_history[instrument][order_index] = fp_i;
			/* Work done. */
			global_context->worker_data[id].go = 0;
		}
	}
	
	dbg_threading("Thread=%d DONE.\n", id);
}

spinlock_t file_lock;

//TODO move to c++ class once done
void Initialize(context_t *my_context)
{
	/* Init basic variables. */
	my_context->global_id = 0;
	my_context->running = 1;
	my_context->bid_order_index = 0;
	my_context->ask_order_index = 0;
	my_context->order_index = 0;
	int ret = pthread_barrier_init(&tmp_barrier, NULL,
	                               THREAD_COUNT + 1);
	assert(ret == 0);
	
	/* Init worker structures. */
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
		my_context->worker_data[i].go = 0;
	}

	/* Init spinlocks. */
	spinlock_init(&file_lock, PTHREAD_PROCESS_SHARED);
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
		spinlock_init(&(my_context->insert_instrument_lock[i]),
		              PTHREAD_PROCESS_SHARED);
		my_context->fp_i_history[i] = malloc(128 * sizeof(int));
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
			malloc(AVAILABLE_MEMORY / (INSTRUMENT_COUNT * 2));
		assert(my_context->asks[i].data);
		memset(my_context->asks[i].data, 0,
		       AVAILABLE_MEMORY / (INSTRUMENT_COUNT * 2));
		memset(my_context->asks[i].history, 0, 128 * sizeof(int));
		my_context->asks[i].vol_ask = 0;
		my_context->asks[i].price_ask = 99999;
		my_context->bids[i].data =
			malloc(AVAILABLE_MEMORY / (INSTRUMENT_COUNT * 2));
		assert(my_context->bids[i].data);
		memset(my_context->bids[i].data, 0,
		       AVAILABLE_MEMORY / (INSTRUMENT_COUNT * 2));
		memset(my_context->asks[i].history, 0, 128 * (sizeof(int) * 2));
		my_context->bids[i].price_bid = 0;
		my_context->bids[i].vol_bid = 0;
	}
	
	memset(my_context->order_ask, 0, 128);
	memset(my_context->order, 0, 128);
	memset(my_context->order_bid, 0, 128);
	
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
	dbg_calc("Update ID=%d called. (context=%p)\n",
	         seqNum, my_context);
	/* prozatim takto, ale mozna na to bude lepsi extra thread. I kdyz asi ne. */
	int *pointer = my_context->order + ((my_context->order_index) % 128); //;//__sync_fetch_and_add(&(context->order_index), 1);
	//pokud nastavim na tom indexu seqnum na 1, tak je tento seqnum uz hotovy
	*pointer = 0;
	/* Find available thread to give work to. Blocks if all threads are full, at least for now. */
	dbg_calc("Getting free thread ID.\n");
	int id = -1;
	while (id < 0) {
		for (int i = 0; i < THREAD_COUNT; i++) {
			if (my_context->worker_data[i].go == 0) {
				id = i;
			}
		}
	}
	/* Fill thread's data. Computation will start automatically. */
//	memcpy(&(my_context->worker_data[id]), &tmp_data, sizeof(tmp_data));
	my_context->worker_data[id].seqNum = seqNum;
	my_context->worker_data[id].instrument = instrument;
	my_context->worker_data[id].side = side;
	my_context->worker_data[id].volume = volume;
	my_context->worker_data[id].price = price;
	my_context->worker_data[id].go = 1;
	my_context->worker_data[id].order_index = my_context->order_index % 128;
	dbg_calc("Starting computation with thread=%d\n", id);
	my_context->order_index++;
}


void Result(context_t *context,
            int seqNum, double fp_global_i)
{
	/* Will print everything to a file. */
	dbg_test("Result called: %d, %f\n", seqNum, fp_global_i);
}
