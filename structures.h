#ifndef STRUCTURES_H
#define STRUCTURES_H

enum rsj_consts {
	INSTRUMENT_COUNT = 20
};

struct rsj_data {
	//Update part
	int seqNum;
	int instrument;
	int price;
	int volume;
	int side;
	//Helper variables
	int volBid_i;
	int priceBid_i;
        int volAsk_i;
	int priceAsk_i;
	double equity_i;
        double f_i;
        double g_i;
        double fp_i;
	//Result part
	double fp_global_i;
};

typedef struct rsj_data rsj_data_t;

struct context {
	double g_array[INSTRUMENT_COUNT];
	double f_array[INSTRUMENT_COUNT];
	double price_bid_array[INSTRUMENT_COUNT];
	double fp_array[INSTRUMENT_COUNT];
};

typedef struct context context_t;


#endif // STRUCTURES_H
