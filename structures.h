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
	char order_index;
	int go;
};

typedef struct rsj_data_in rsj_data_in_t;

struct rsj_double {
	int vol_bid;
	int price_bid;
};

typedef struct rsj_double rsj_double_t;

struct lookup_bid {
	int vol_bid;
	int price_bid;
	int *data;
	rsj_double_t history[128];
};

typedef struct lookup_bid lookup_bid_t;

struct lookup_ask {
	int vol_ask;
	int price_ask;
	int *data;
	int history[128];
};

typedef struct lookup_ask lookup_ask_t;

struct context {
	/* Testing part. */
	double g_array[INSTRUMENT_COUNT];
	double f_array[INSTRUMENT_COUNT];
	double price_bid_array[INSTRUMENT_COUNT];
	double fp_i_array[INSTRUMENT_COUNT];
	double best_bid[INSTRUMENT_COUNT];
	/* Normal part. */
//	pthread_t scheduler_thread;
	spinlock_t insert_instrument_lock[INSTRUMENT_COUNT];
	pthread_barrier_t global_worker_sync;
	int running = 1;
	int global_id = 0;
	spinlock_t sum_lock;
	double fp_i_sum = 0.0;
//	minmax_t minmax;
	lookup_bid_t bids[INSTRUMENT_COUNT];
	lookup_ask_t asks[INSTRUMENT_COUNT];
	pthread_t worker_threads[THREAD_COUNT];
	rsj_data_in_t worker_data[INSTRUMENT_COUNT];
	/* Overflow OK. */
	char order_index = 0;
	int order[128];
};

typedef struct context context_t;

#endif // STRUCTURES_H
