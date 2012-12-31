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

//can be in this file for now, once this becomes C++, it need to move
void Initialize(context_t *my_context);
void Update(context_t *context,
            int seqNum, int instrument, int price, int volume, int side);
void Result(context_t *context,
            int seqNum, double fp_global_i, int index);

#endif // TESTER_H
