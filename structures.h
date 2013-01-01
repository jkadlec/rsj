#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <pthread.h>
#include <malloc.h>
#include <ck_ring.h>

#include "rb.h"
#include "iresultconsumer.h"

#define THREAD_COUNT 1
#define HISTORY_SIZE 8
#define SPECIFIC_HISTORY_SIZE 8
#define BUFFER_SIZE 2

#define MEASURE_TIME

#define TESTING_COUNT 1000

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
	unsigned int order_index;
	unsigned int specific_index;
	char PAD[64 - (sizeof(int) * 7)];
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
	rb_table_t *tree;
	int history[SPECIFIC_HISTORY_SIZE];
};

typedef struct lookup_ask lookup_ask_t;

struct context {
	int running;
	int global_id;
	lookup_bid_t bids[INSTRUMENT_COUNT];
	lookup_ask_t asks[INSTRUMENT_COUNT];
	pthread_t worker_threads[THREAD_COUNT];
	unsigned int order_index;
	unsigned int order_indices_bid[INSTRUMENT_COUNT];
	int order[HISTORY_SIZE];
	int order_sum[HISTORY_SIZE];
	double sum_history[HISTORY_SIZE];
	double *fp_i_history[INSTRUMENT_COUNT];
	ck_ring_t *buffer;
};

typedef struct context context_t;
     
#endif // STRUCTURES_H
