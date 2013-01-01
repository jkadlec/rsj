#include <pthread.h>

#include "globals.h"
#include "structures.h"
#include "iresultconsumer.h"

pthread_barrier_t start_barrier;
context_t *global_context = (context_t *)malloc(sizeof(context_t));
IResultConsumer consumer;

#ifdef MEASURE_TIME
struct timespec times[HISTORY_SIZE];
#endif
