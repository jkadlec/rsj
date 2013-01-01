#ifndef IUPDATEPROCESSOR_H
#define IUPDATEPROCESSOR_H

#include "iresultconsumer.h"

class IUpdateProcessor
{
	public:
		virtual void Initialize(IResultConsumer& resultConsumer);
		virtual void Update(int seqNum, int instrument, int price,
		                    int volume, int side);
	virtual ~IUpdateProcessor();
};

#endif // IUPDATEPROCESSOR_H
