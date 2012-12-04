#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "scheduler.h"
#include "tester.h"
#include "structures.h"
#include "debug.h"

int main(int argc, char **argv)
{
	/* Load testing .csv */
	rsj_data_t **data = NULL;
	size_t data_count = 0;
	int ret = tester_load_csv_file(argv[1], &data,
	                               &data_count);
	assert(ret);
	
	/* Feed the data to a testing structure. */
	
	/* Start the computation. */
	
	/* Check the results. */
}


