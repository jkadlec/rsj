#ifndef WORKER_H
#define WORKER_H

#include <time.h>
#include <stdlib.h>

#include "structures.h"
#include "globals.h"
#include "debug.h"

void *worker_start(void *null_data);

inline void update_c_style(int seqNum, int instrument, int price, int volume,
                           int side)
{
	struct timespec time;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
	rsj_data_in_t *data = (rsj_data_in_t *)malloc(sizeof(rsj_data_in_t));
	data->seqNum = seqNum;
	data->instrument = instrument;
	data->side = side;
	data->volume = volume;
	data->price = price;
	data->order_index = (global_context->order_index++) % HISTORY_SIZE;
	data->specific_index = (global_context->order_indices_bid[instrument]++) % SPECIFIC_HISTORY_SIZE;
	times[global_context->order_index] = time;
	while (!ck_ring_enqueue_spmc(global_context->buffer, (void *)data)) {
		;
	}
	dbg_ring("Enqueued seqNum=%d\n", seqNum);
}

void destructor_c_style();


#endif // WORKER_H
