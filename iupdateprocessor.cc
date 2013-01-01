#include "iupdateprocessor.h"
#include "structures.h"
#include "tester.h"

void IUpdateProcessor::Initialize(IResultConsumer& resultConsumer)
{
	consumer = resultConsumer;
	initialize_c_style();
}

void IUpdateProcessor::Update(int seqNum, int instrument, int price,
            int volume, int side)
{
	update_c_style(seqNum, instrument, price, volume, side);
}

IUpdateProcessor::~IUpdateProcessor()
{
	destructor_c_style();
}
