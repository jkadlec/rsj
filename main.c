#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "scheduler.h"
#include "tester.h"
#include "structures.h"
#include "debug.h"

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
	my_context.results = data;
	Initialize(&my_context);
	/* Start the computation. */
	do_test(&my_context, data, 30);
	/* Check the results. */
	
	getchar();
	
	return 0;
}


