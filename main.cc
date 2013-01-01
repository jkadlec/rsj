#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "worker.h"
#include "structures.h"
#include "debug.h"
#include "iupdateprocessor.h"
#include "globals.h"

int tester_load_csv_file(const char *filename, rsj_data_t ***data,
                         size_t *data_count)
{
	FILE *f = fopen(filename, "r");
	assert(f);
	
	dbg_test("Loading file=%s\n",
	         filename);
	
	*data = (rsj_data_t **)malloc(TESTING_COUNT * sizeof(rsj_data_t **));
	assert(data);
	
	/* Read line by line. */
	rsj_data_t tmp_data;
	int ret = 0;
	size_t i = 0;
	while ((ret = fscanf(f, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%lf,%lf,%lf,%lf,%lf\n",
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
		(*data)[i] = (rsj_data_t *)malloc(sizeof(rsj_data_t));
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
		if ((*data)[i]->seqNum == 1001) {
			break;
		}
		i++;
	}
	
	*data_count = ++i;
	loaded = *data_count;
	return 0;
}


void do_test(IUpdateProcessor *proc, rsj_data_t **data, size_t count)
{
	dbg_test("Testing %d queries.\n",
	         count);
	gettimeofday(&start_time, NULL);
	for (size_t i = 0; i < count; i++) {
		proc->Update(data[i]->seqNum, data[i]->instrument,
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
	IUpdateProcessor *proc = new IUpdateProcessor();
	proc->Initialize(consumer);
	/* Start the computation. */
	do_test(proc, data, data_count);
	
	sleep(30);
	
	return 0;
}


