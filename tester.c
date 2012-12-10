#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "debug.h"
#include "error.h"
#include "tester.h"
#include "structures.h"

#define THREAD_COUNT 2;

#define AVAILABLE_MEMORY 2 * 1024 * 1000

//instrumenty se muzou vkladat do struktury paralelne, ale vlozeni pro instrumenty se musi serializovat - jak?
//pred vracenim fp_i_total musim vedet, ze vlakna predemnou uz dobehla a maji zapsano v fp_sum, stejne tak jsou potreba predchozi fp_i pro vsechna asi, ale to muze byt nejak ulozene pospolu
//pro kazdy seqnum potrebuju:
//ceny budou stacit jedny globalni, ty se upravuji je pro stejne instrumenty, ale jako historie bude stacit ten nejlepsi a nejhorsi, to se porovna s tim, co tam vkladam ja a je to. Jen v pripade, ze to odstranuju, se bude muset neco dit a to bude muset mit sakra prioritu, jinak to bude pomale.
//				vlozit, ceny, volumy, fp_n

#define spinlock_init pthread_spin_init
#define spinlock_lock pthread_spin_lock
#define spinlock_unlock pthread_spin_unlock

void bids_insert(lookup_bid_t *bids, spinlock_t *instrument_lock,
                 int history_index, int vol_bid, int price_bid)
{
	/* Insert into array. */
	assert(bids[price_bid] == 0);
	//toto nebude fungovat, ale oh well.
	spinlock_lock(instrument_lock);
	bids[price_bid] = vol_bid;
	if (price_bid > bids->price_bid) {
		bids->price_bid = price_bid;
		bids->vol_bid = vol_bid;
	}
	
	bids->history[history_index].price_bid = bids->price_bid;
	bids->history[history_index].vol_bid = bids->vol_bid;
	//toto nebude fungovat, ale oh well.
	spinlock_lock(instrument_lock);
}

void asks_insert(lookup_bid_t *asks, spinlock_t *instrument_lock,
                 int history_index, int vol_ask, int price_ask)
{
	/* Insert into array. */
	assert(bids[price_ask] == 0);
	//toto nebude fungovat, ale oh well.
	spinlock_lock(instrument_lock);
	bids[price_ask] = vol_ask;
	if (price_ask < bids->price_ask) {
		asks->price_ask = price_ask;
		asks->vol_ask = vol_ask;
	}
	
//	asks->history[history_index].price_bid = bids->price_bid;
	asks->history[history_index] = bids->vol_ask;
	spinlock_unlock(instrument_lock);
}

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

void sums_add_fp_i(lookup_t *sums, int seqNum, double fp_i)
{
	double fp_i_previous = sums
}

inline double compute_fp_global(lookup_t *sums, int seqNum, double fp_i)
{
	return fp_i / sums_lookup_sum(sums, seqNum);
}

void worker_start(void *data)
{
	context_t *global_context = (context_t *)data;
	/* Create private thread context. */
	worker_context_t thread_context;
	/* Obtain id. */
	int id = __sync_fetch_and_add(&global_context->global_id, 1);
	dbg_threading("Thread aquired ID=%d\n", id);
	/* Sync with other worker threads. */
	pthread_barrier_wait(global_context->global_worker_sync);
	/* While computation not done */
	while (global_context->running) {
		/* Wait for work. */
		if (global_context->worker_data[id].go) {
			/* Get work. */
			rsj_data_in_t in_data;
			int instrument =
				global_context->worker_data[i].instrument;
			int seqNum =
				global_context->worker_data[i].seqNum;
			int price =
				global_context->worker_data[i].seqNum;
			int side =
				global_context->worker_data[i].side;
			int volume =
				global_context->worker_data[i].volume;
			int volbid = 0;
			int volask = 0;
			int pricebid = 0;
			if (side == SIDE_BUYING) {
				/* Lock instrument lock. */
//				spinlock_lock(global_context->fp_i_locks[instrument]);
				/* Insert this update to lookup structure. */
				lookup_buy_insert(global_context->buy_lookups[instrument],
				                  seqNum, price, volume);
			} else if (side == SIDE_SELLING) {
//				spinlock_lock(global_context->fp_i_locks[instrument]);
				lookup_sell_insert(global_context->sell_lookuos[instrument],
				                   seqNum, price);
			}
			if (unlikely(price == PRICE_CANCEL)) {
				continue;
			}
			/* Get the best buying price. */
			int pricebid = lookup_buy_price(global_context->buy_lookups[instrument], seqNum);
			/* Get the best price volume. */
			int volbid = lookup_buy_volume(global_context->buy_lookups[instrument], seqNum);
			/* Get the best ask volume. */
			int volask = lookup_ask_volume(global_context->ask_lookups[instrument], seqNum);
			/* Unlock instrument lock. */
//			spinlock_unlock(global_context->fp_i_locks[in_data.instrument]);
			
			/* Do work. Only compute fp_i. */
			double fp_i = compute_fp_i(volbid / (volbid + volask),
			                           pricebid);
			
			/* Change fp_sum accordingly. */
			sums_add_fp_i(global_context->fp_sums, seqNum, fp_i);
			
			/* Work done. */
			global_context->worker_data[id].go = 0;
			Result(seqNum, compute_fp_global(global_context->fp_sums,
			                                 seqNum, fp_i));
		}
	
		/* Call worker function. */
	
		/* Tell scheduler that this thread can work again. */
		global_context->[];
	}
}

spinlock_t file_lock;

//TODO move to c++ class once done
void Initialize()
{
	context_t my_context;
	pthread_barrier_init(my_context.global_worker_sync, NULL, THREAD_COUNT);
	
	/* Start scheduler thread. */
	//might not be needed
//	pthread_create(context->scheduler_thread, NULL, scheduler_thread_func,
//	               context);
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
		spinlock_init(&(my_context.insert_instrument_lock[i]),
		              SHARED);
	}
	
	spinlock_init(&file_lock, SHARED);
	/* Start worker threads. */
	for (int i = 0; i < THREAD_COUNT; i++) {
		pthread_create(my_context.worker_threads[i],
		               NULL, worker_start, &my_context);
	}
	
	/* Init lookup structures. */
	for (int i = 0; i < INSTRUMENT_COUNT; i++) {
		//toto bude fungovat, ale bidy potrebujou vic dat nez asky
		my_context.asks[i].data =
			malloc(AVAILABLE_MEMORY / (INSTRUMENT_COUNT * 2));
		memset(my_context.asks[i].data, 0,
		       AVAILABLE_MEMORY / (INSTRUMENT_COUNT * 2));
		my_context.asks[i].vol_ask = 0;
		my_context.bets[i].data =
			malloc(AVAILABLE_MEMORY / (INSTRUMENT_COUNT * 2));
		memset(my_context.bets[i].data, 0,
		       AVAILABLE_MEMORY / (INSTRUMENT_COUNT * 2));
		my_context.bids[i].price_bid = 0;
		my_context.bids[i].vol_bid = 0;
	}
}

int pool_return_free_thread_id(rsj_data_in_t *worker_data)
{
	while (1) {
		for (int i = 0; i < INSTRUMENT_COUNT; i++) {
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
	/* prozatim takto, ale mozna na to bude lepsi extra thread. I kdyz asi ne. */
	int *pointer = my_context->order + ((my_context->order_index++) % 128); //;//__sync_fetch_and_add(&(context->order_index), 1);
	//pokud nastavim na tom indexu seqnum na 0, tak je tento seqnum uz hotovy
	*pointer = 0;
	rsj_data_in_t tmp_data;
	tmp_data.seqNum = seqNum;
	tmp_data.instrument = instrument;
	tmp_data.volume = volume;
	tmp_data.side = side;
	tmp_data.go = 1;
	/* Find available thread to give work to. Blocks if all threads are full, at least for now. */
	int id = pool_return_free_thread_id(my_context->worker_data);
	/* Fill thread's data. Computation will start automatically. */
	memcpy(&(my_context->worker_data), &tmp_data, sizeof(tmp_data));
}


void Result(context_t *context,
            int seqNum, double fp_global_i)
{
	/* Will print everything to a file. */
	
}
