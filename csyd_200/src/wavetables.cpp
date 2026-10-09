
#include "ss.h"
#include <math.h>

ssfloat				*gSinTable,*gSquareTable;

void InitWaveTables()
{
	long	i;
	int		h;
	ssfloat	hTime,hx;
	ssfloat	hScale;
	double	max = 0.0;

	if (gSinTable != NULL && gSquareTable != NULL)
			return;
	LogMessage("Initializing wave tables\n");

	gSinTable = (ssfloat *) MyNewPtrClear(sizeof(ssfloat)*WaveTableSize);
	gSquareTable = (ssfloat *) MyNewPtrClear(sizeof(ssfloat)*WaveTableSize);

	hTime = pi*2/(double) WaveTableSize;
	for (i = 0,hx = 0; i < WaveTableSize; ++i, hx += hTime) {
		gSinTable[i] = sssin(hx);
		gSquareTable[i] = 0;
	}

	for (i = 0; i < WaveTableSize; ++i)
		gSquareTable[i] = 0;

	// 1 + 3x + 5x + 7x + 9x
	// f1 = f*2*pi/(double)WaveTableSize
	for (h = 1; h <= 9; h += 2) {	// was 9
		// hTime = (2*pi*h)/(double) WaveTableSize;
		hScale = 1.0 / h;
		for (i = 0; i < WaveTableSize; ++i) {
			gSquareTable[i] += hScale*sssin((i*2*pi*h)/(double) WaveTableSize);
		}
	}
/* attempt to find simplified method from Moore pg 273
	{
		double	piF;
		double	n = 10;
		piF = 2*pi/(double)WaveTableSize;
		for (i = 0; i < WaveTableSize; ++i) {
			gSquareTable[i] =pow(sssin(n*i*piF),2.0)/sssin(i*piF);
		}
	}
*/
	// Find Maximum for Normalization
	for (i = 0; i < WaveTableSize; ++i) {
		if (fabs(gSquareTable[i]) > max)
			max = gSquareTable[i];
	}
	// Normalize
	// max = 1/max;
	for (i = 0; i < WaveTableSize; ++i) {
		gSquareTable[i] /= max;
	}
}

