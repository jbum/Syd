// UserSounds.h
#if macintosh

#include <Sound.h>
pascal void MyCallBack(SndChannelPtr chan, SndCommand *cmd);

#endif

#define MaxSndBufSamples		(44100L*2)
#define MaxSndBuffers			8

void EndSounds();
// void PlaySound(short i, short priority);
void InitSounds(void);
void SSPlayBuffer(unsigned char *buffer);
void SSShutUp();
int GetQueuedSoundBuffers();
Ptr GetSndBuffer();
