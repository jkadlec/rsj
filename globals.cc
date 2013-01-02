#include <pthread.h>

#include "globals.h"
#include "structures.h"
#include "iresultconsumer.h"

pthread_barrier_t start_barrier;
context_t *global_context;
IResultConsumer consumer;

#ifdef MEASURE_TIME
struct timeval start_time;
struct timespec times[HISTORY_SIZE];
size_t loaded;
#endif
