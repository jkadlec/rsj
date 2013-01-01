#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "debug.h"
#include "helpers.h"
#include "sync.h"
#include "structures.h"

inline void bids_insert(lookup_bid_t *bids,
                        unsigned int history_index, unsigned int specific_index,
                        int vol_bid, int price_bid)
{
	dbg_calc("Thread with history index=%d: Inserting bid, volBid=%d,"
	         "priceBid=%d\n",
	         specific_index, vol_bid, price_bid);
	/* Wait until previous data is inserted. */
	unsigned int index = prev_index(history_index);
	dbg_calc("From %d to %d\n", history_index, index);
dbg_threading_exec(
	dbg_threading("Order:\n");
	for (int i = 0; i < HISTORY_SIZE; i++) {
		dbg_threading("%d=%d ", i, global_context->order[i]);
	}
	dbg_threading("\n");
);
	dbg_calc("%d:Waiting for thread with order=%d to finish with adding.\n",
	         specific_index, index);
	wait_until_set_order(index);
	sync_unset_order(history_index);
	dbg_calc("%d:All good, free to go.\n", specific_index);
	rsj_pair_t *pair = (rsj_pair_t *)malloc(sizeof(rsj_pair_t));
	assert(pair);
	pair->price_bid = price_bid;
	pair->vol_bid = vol_bid;
	rsj_pair_t *ret = (rsj_pair_t *)rb_replace(bids->tree, pair);
	if (ret != NULL) {
		free(ret);
	}
	if (unlikely(vol_bid == 0)) {
		dbg_calc("Thread with history index=%d: Removing price.\n",
			specific_index);
		rsj_pair_t *ret = (rsj_pair_t *)rb_delete(bids->tree, pair);
		free(ret);
		if (price_bid == bids->price_bid) {
			dbg_calc("Thread with history index=%d: Was best price.\n",
				specific_index);
			struct rb_traverser trav;
			rsj_pair_t *new_max = (rsj_pair_t *)rb_t_last(&trav, bids->tree);
			if (unlikely(new_max == NULL)) {
				dbg_calc("No better price.\n");
				bids->price_bid = 0;
				bids->vol_bid = 0;
				bids->history[specific_index].price_bid = 0;
				bids->history[specific_index].vol_bid = 0;
			} else {
				dbg_calc("Thread with history index=%d: New best price=%d,"
				         " volume=%d. (used to be %d)\n",
				         specific_index, index, new_max->vol_bid,
				         price_bid);
				bids->price_bid = new_max->price_bid;
				bids->vol_bid = new_max->vol_bid;
				bids->history[specific_index].price_bid =
					new_max->price_bid;
				bids->history[specific_index].vol_bid =
					new_max->vol_bid;
			}
		} else {
			bids->history[specific_index].price_bid =
				bids->history[prev_index_spec(specific_index)].price_bid;
			bids->history[specific_index].vol_bid =
				bids->history[prev_index_spec(specific_index)].vol_bid;
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
		bids->history[specific_index].price_bid = bids->history[prev_index_spec(specific_index)].price_bid;
		bids->history[specific_index].vol_bid = bids->history[prev_index_spec(specific_index)].vol_bid;
	}
	
	/* Tell other threads that this thread is done with insertion. */
	sync_set_order(history_index);
	dbg_threading("Order=%d unlocked.\n", history_index);
	dbg_calc("Thread with history index=%d: Price after insert=%d, volume=%d\n",
	         history_index,
	         bids->history[specific_index].price_bid,
	         bids->history[specific_index].vol_bid);
}

inline void asks_insert(lookup_ask_t *asks,
                 unsigned int history_index, unsigned int specific_index,
                 int vol_ask, int price_ask)
{
	dbg_calc("Thread with history index=%d: Inserting ask, volAsk=%d,"
	         "priceAsk=%d (current best = %d %d)\n",
	          specific_index, vol_ask, price_ask, asks->price_ask, 
	         asks->vol_ask);
	unsigned int index = prev_index(history_index);
	dbg_calc("%d:Waiting for thread with order=%d to finish with adding.\n",
			history_index, index);
	wait_until_set_order(index);
	sync_unset_order(history_index);
	dbg_calc("%d:All good, free to go.\n", specific_index);
	
	rsj_pair_t *pair = (rsj_pair_t *)malloc(sizeof(rsj_pair_t));
	assert(pair);
	pair->price_bid = price_ask;
	pair->vol_bid = vol_ask;
	rsj_pair_t *ret =(rsj_pair_t *) rb_replace(asks->tree, pair);
	if (ret != NULL) {
		free(ret);
	}
	
	if (unlikely(vol_ask == 0)) {
		dbg_calc("Thread with history index=%d: Removing price.\n",
		         specific_index);
		rsj_pair_t *ret = (rsj_pair_t *) rb_delete(asks->tree, pair);
		free(ret);
		if (price_ask == asks->price_ask) {
			dbg_calc("Thread with history index=%d: Was best price.\n",
			         specific_index);
                        struct rb_traverser trav;
                        rsj_pair_t *new_min = (rsj_pair_t *) rb_t_first(&trav, asks->tree);
			if (unlikely(new_min == NULL)) {
				dbg_calc("No better price.\n");
				asks->price_ask = INT_MAX;
				asks->vol_ask = 0;
				asks->history[specific_index] = 0;
			} else {
				dbg_calc("Thread with history index=%d: New best price=%d, volume=%d.\n",
				         specific_index, index, new_min->vol_bid);
				asks->price_ask = new_min->price_bid;
				asks->vol_ask = new_min->vol_bid;
				asks->history[specific_index] = new_min->vol_bid;
			}
		} else {
			asks->history[specific_index] =
				asks->history[prev_index_spec(specific_index)];
		}
	} else if (price_ask <= asks->price_ask) {
		dbg_calc("Thread with history index=%d: new best price (ask).\n",
		         specific_index);
		asks->price_ask = price_ask;
		asks->vol_ask = vol_ask;
		asks->history[specific_index] = vol_ask;
	} else {
		dbg_calc("%d: no change, using history.\n", specific_index);
		asks->history[specific_index] = asks->history[prev_index(specific_index)];
	}
	
	/* Tell other threads that this thread is done with insertion. */
	sync_set_order(history_index);
	dbg_threading("Order=%d unlocked.\n", history_index);
	dbg_calc("Thread with history index=%d: Price after insert = %d, volask=%d\n",
	         specific_index, asks->price_ask,
	         asks->history[specific_index]);

}

#endif // TABLE_H
