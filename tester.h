#ifndef TESTER_H
#define TESTER_H

#include "structures.h"

int tester_load_csv_file(const char *filename, rsj_data_t ***data,
                         size_t *data_count);

int tester_start_computation(rsj_data_t **data,
                             size_t data_count,
                             void *params);

int tester_check_results(rsj_data_t *theirs, rsj_data_t *ours,
                         size_t data_count);

void Update(int seqNum, int instrument, int price, int volume, int side);
void Result(int seqNum, double fp_global_i);

#endif // TESTER_H
