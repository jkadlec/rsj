#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <pthread.h>

#define THREAD_COUNT 4
#define HISTORY_SIZE 8
#define SPECIFIC_HISTORY_SIZE 8

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
	char specific_index;
	char go;
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
	rsj_double_t history[1280];
};

typedef struct lookup_bid lookup_bid_t;

struct lookup_ask {
	int vol_ask;
	int price_ask;
	int *data;
	int history[1280];
};

typedef struct lookup_ask lookup_ask_t;

struct context {
	pthread_barrier_t *global_worker_sync;
	char running;
	char global_id;
	lookup_bid_t bids[INSTRUMENT_COUNT];
	lookup_ask_t asks[INSTRUMENT_COUNT];
	pthread_spinlock_t instrument_locks[HISTORY_SIZE];
	pthread_spinlock_t sum_locks[HISTORY_SIZE];
	pthread_t worker_threads[THREAD_COUNT];
	rsj_data_in_t worker_data[INSTRUMENT_COUNT];
	/* Overflow OK. */
	char order_index;
	unsigned char order_indices_bid[INSTRUMENT_COUNT];
	int order[128];
	double sum_history[128];
	double *fp_i_history[INSTRUMENT_COUNT];
};

typedef struct context context_t;

#endif // STRUCTURES_H
