// SSNoise.cp
//
// Fix to return predictable values based on time?

#include "ss.h"
#include "ssnoise.h"

SSNoise::SSNoise(ModList * mList, short h, short v) : SSModule(MT_Noise, mList, h, v)
{
	startSeed = 1;
	randomize = true;
}

ssfloat SSNoise::GenerateOutput(SSModule *callingMod)
{
	ssfloat retVal = DoubleRandom();
  lastRightSample = retVal;
  return retVal;
}



void	SSNoise::Copy(SSModule *ss)
{
	SSNoise *sa = (SSNoise *) ss;
	SSModule::Copy(ss);
	startSeed = sa->startSeed;
	randomize = sa->randomize;
}

/*
void SSNoise::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"RND %d %d\r",
		(randomize? 1 : 0), startSeed);
}
*/

void SSNoise::Load(FILE* ar) 
{
	char				*p,tbuf[256];
	int					rnd=1,seed=0;

	p = parList->GetNextInputLine(ar,"RND",tbuf);
	sscanf(p, "RND %d %d",&rnd,&seed);
	randomize = (rnd > 0);
	startSeed = seed;
}

long SSNoise::LongRandom(void)
{
	int32	hi,lo,test;

	hi   = seed / R_Q;
	lo   = seed % R_Q;
	test = R_A * lo - R_R * hi;
	if (test > 0)
    seed = test;
	else
	  seed = test + R_M;
	return seed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SSNoise::MySRand(int32 s)
{
	seed = s;
	if (seed == 0)
		seed = 1;
}

void SSNoise::Randomize(void)
{
	MySRand(time(NULL));
}

ssfloat SSNoise::DoubleRandom(void)
{
	return LongRandom() / (ssfloat) R_M;
}

void SSNoise::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	if (randomize)
		Randomize();
	else
		MySRand(startSeed);
}
