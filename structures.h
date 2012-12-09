#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <pthread.h>

#define spinlock_t pthread_spinlock_t

enum rsj_consts {
	INSTRUMENT_COUNT = 20
};

struct rsj_data {
	//Update part
	int seqNum;
	int instrument;
	int price;
	int volume;
	int side;
	//Helper variables
	int volBid_i;
	int priceBid_i;
        int volAsk_i;
	int priceAsk_i;
	double equity_i;
        double f_i;
        double g_i;
        double fp_i;
	//Result part
	double fp_global_i;
};

typedef struct rsj_data rsj_data_t;

struct rsj_data_in {
	//Update part
	int seqNum;
	int instrument;
	int price;
	int volume;
	int side;
};

typedef struct rsj_data_in rsj_data_in_t;

struct context {
	double g_array[INSTRUMENT_COUNT];
	double f_array[INSTRUMENT_COUNT];
	double price_bid_array[INSTRUMENT_COUNT];
	double fp_array[INSTRUMENT_COUNT];
	double *best_prices[INSTRUMENT_COUNT];
	rsj_data_in_t in_array[1024];
	size_t data_count;
	pthread_t scheduler_thread;
	spinlock_t fp_i_locks[INSTRUMENT_COUNT];
	pthread_barrier_t global_worker_sync;
	int running = 0;
	int global_id;
};

typedef struct context context_t;

#endif // STRUCTURES_H
