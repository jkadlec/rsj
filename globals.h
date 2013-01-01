#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include "structures.h"
#include "iresultconsumer.h"

extern pthread_barrier_t start_barrier;
extern context_t *global_context;
extern IResultConsumer consumer;

#endif // GLOBALS_H
