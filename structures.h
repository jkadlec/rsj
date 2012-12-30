#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <pthread.h>

#include "rb.h"

#define THREAD_COUNT 1
#define HISTORY_SIZE 4
#define SPECIFIC_HISTORY_SIZE 4

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

struct rsj_pair {
	int vol_bid;
	int price_bid;
};

typedef struct rsj_pair rsj_pair_t;

struct lookup_bid {
	int vol_bid;
	int price_bid;
	rb_table_t *tree;
	rsj_pair_t history[SPECIFIC_HISTORY_SIZE];
};

typedef struct lookup_bid lookup_bid_t;

struct lookup_ask {
	int vol_ask;
	int price_ask;
	int *data;
	rb_table_t *tree;
	int history[SPECIFIC_HISTORY_SIZE];
};

typedef struct lookup_ask lookup_ask_t;

struct context {
	pthread_barrier_t *global_worker_sync;
	char running;
	char global_id;
	lookup_bid_t bids[INSTRUMENT_COUNT];
	lookup_ask_t asks[INSTRUMENT_COUNT];
	pthread_t worker_threads[THREAD_COUNT];
	rsj_data_in_t worker_data[INSTRUMENT_COUNT];
	/* Overflow OK. */
	char order_index;
	unsigned char order_indices_bid[INSTRUMENT_COUNT];
	int order[HISTORY_SIZE];
	int order_sum[HISTORY_SIZE];
	double sum_history[HISTORY_SIZE];
	double *fp_i_history[INSTRUMENT_COUNT];
};

typedef struct context context_t;

#endif // STRUCTURES_H
