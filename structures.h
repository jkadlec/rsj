#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <pthread.h>
#include <ck_spinlock.h>
#include <ck_ring.h>

#include "rb.h"
#include "iresultconsumer.h"

#define THREAD_COUNT 3
#define HISTORY_SIZE 128
#define SPECIFIC_HISTORY_SIZE 128
#define BUFFER_SIZE 4

#define MEASURE_TIME

#define SPIN
#define DEC
//#define ATOMIC

#ifdef NORMAL
#define lock_t pthread_spinlock_t
#define lock_init pthread_spin_init
#define lock_lock pthread_spin_lock
#define lock_unlock pthread_spin_unlock
#endif

#ifdef DEC
#define lock_t ck_spinlock_dec_t
#define lock_locked ck_spinlock_dec_locked
#define lock_lock ck_spinlock_dec_lock_eb
#define lock_trylock ck_spinlock_dec_trylock
#define lock_unlock ck_spinlock_dec_unlock
#endif

#define TESTING_COUNT 6000002

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
#ifdef ATOMIC
	int order[HISTORY_SIZE];
	int order_sum[HISTORY_SIZE];
#endif
#ifdef SPIN
	lock_t order[HISTORY_SIZE];
	lock_t order_sum[HISTORY_SIZE];
#endif
	double sum_history[HISTORY_SIZE];
	double *fp_i_history[INSTRUMENT_COUNT];
	ck_ring_t *buffer;
};

typedef struct context context_t;
     
#endif // STRUCTURES_H
