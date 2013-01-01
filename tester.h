#ifndef TESTER_H
#define TESTER_H

#include "structures.h"

int tester_load_csv_file(const char *filename, rsj_data_t ***data,
                         size_t *data_count);

int tester_start_computation(rsj_data_t **data,
                             size_t data_count,
                             void *params);

//can be in this file for now, once this becomes C++, it need to move
void initialize_c_style();
void update_c_style(int seqNum, int instrument, int price, int volume, int side);
void result_c_style(int seqNum, double);
void destructor_c_style();

#endif // TESTER_H
