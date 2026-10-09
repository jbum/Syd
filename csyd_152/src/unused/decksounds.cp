/************************************************************************************
 * User.c
 *
 ************************************************************************************/

#include "SS.h"
#include "DeckSounds.h"
#include "SSSound.h"


static SndChannelPtr	scp;
static SndCommand		tCmd;						/* Sound Command */
SndCallBackUPP			mySndCallBack;
static Ptr				sndBufs[3];
static int				sndBufCtr;

pascal void MyCallBack(SndChannelPtr chan, SndCommand *cmd)
{
	chan->userInfo--;
	if (chan->userInfo < 0)
		chan->userInfo = 0;
}

void InitSounds()
{
	OSErr	oe;
	int		i;
	scp = (SndChannelPtr) MyNewPtrClear((long) sizeof(SndChannel));
	scp->qLength = stdQLength;
	mySndCallBack = NewSndCallBackProc((ProcPtr) MyCallBack);
	if ((oe = SndNewChannel(&scp,sampledSynth,initMono,mySndCallBack)) != noErr)
		DebugStr("\pSound Problem");
	for (i = 0; i < MaxSndBuffers; ++i) 
	{
		sndBufs[i] = (Ptr) MyNewPtrClear(MaxSndBufSamples*sizeof(short)+sizeof(SSExtSoundHeader));
		if (sndBufs[i] == 0)
			ErrorExit("Not Enough Memory for Playback Buffers");
	}
}

void EndSounds()
{
	if (scp) {
		SndDisposeChannel(scp,true);
		DisposPtr((Ptr) scp);
		scp = 0L;
	}
}

/*
void PlaySound(short i, short priority)
{
	if (i < S_NbrSounds && sounds[i] &&
		priority >= scp->userInfo) {
		tCmd.cmd = quietCmd;
		SndDoImmediate(scp,&tCmd);
		tCmd.cmd = flushCmd;
		SndDoImmediate(scp,&tCmd);
#if THINK_C
		SndPlay(scp, sounds[i], true);
#else
		SndPlay(scp, (SndListHandle) sounds[i], true);
#endif
		scp->userInfo = priority;
		tCmd.cmd = callBackCmd;
		SndDoCommand(scp,&tCmd,true);
	}
}
*/

void SSPlayBuffer(unsigned char *buffer)
{
	tCmd.cmd = bufferCmd;
	tCmd.param1 = 0;
	tCmd.param2 = (long) buffer;
	SndDoCommand(scp,&tCmd,true);

	scp->userInfo++;
	tCmd.cmd = callBackCmd;
	tCmd.param1 = 0;
	tCmd.param2 = 0L;
	SndDoCommand(scp,&tCmd,true);

}

void SSShutUp()
{
	tCmd.cmd = quietCmd;
	SndDoImmediate(scp,&tCmd);
	tCmd.cmd = flushCmd;
	SndDoImmediate(scp,&tCmd);
	scp->userInfo = 0;
}

int GetQueuedSoundBuffers()
{
	return (int) scp->userInfo;
}

Ptr GetSndBuffer()
{
	Ptr	s;
	s = sndBufs[sndBufCtr++];
	sndBufCtr %= MaxSndBuffers;
	return s;
}