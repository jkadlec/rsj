#include "iresultconsumer.h"
#include "tester.h"

void IResultConsumer::Result(int seqNum, double fp_global_i)
{
	result_c_style(seqNum, fp_global_i);
}
