#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "scheduler.h"
#include "tester.h"
#include "structures.h"
#include "debug.h"

int tester_load_csv_file(const char *filename, rsj_data_t ***data,
                         size_t *data_count)
{
	FILE *f = fopen(filename, "r");
	assert(f);
	
	dbg_test("Loading file=%s\n",
	         filename);
	
	*data = malloc(102400);
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
	return 0;
}


void do_test(context_t *my_context,
             rsj_data_t **data, size_t count)
{
	dbg_test("Testing %d queries.\n",
	         count);
	for (int i = 0; i < count; i++) {
		Update(my_context, data[i]->seqNum, data[i]->instrument,
		       data[i]->price, data[i]->volume, data[i]->side);
	}
}

int main(int argc, char **argv)
{
	/* Load testing .csv */
	rsj_data_t **data = NULL;
	size_t data_count = 0;
	int ret = tester_load_csv_file(argv[1], &data,
	                               &data_count);
	assert(ret == 0);
	/* Feed the data to a testing structure. */
	context_t my_context;
	Initialize(&my_context);
	/* Start the computation. */
	do_test(&my_context, data, 1000);
	/* Check the results. */
	
//	getchar();
	while (__sync_bool_compare_and_swap(&my_context.running, 1, 1)) {
		;
	}
	
	return 0;
}


