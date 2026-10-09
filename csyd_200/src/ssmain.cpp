#include "ss.h"
#include "patchowner.h"
#include "modlist.h"
#include <ctype.h>
#include <thread>
#include "sspool.h"

void SyntaxExit()
{
  printf("csyd [options] patchfile\n\n");
  printf("Options:\n\n");
  printf("    -o file        output filename (supports .wav and .aif)\n");
  printf("    -d duration    duration (seconds or MM:SS)\n");
  printf("    -s samplerate  sampling rate\n");
  printf("    -v #           verbosity level\n");
  printf("    -q             quiet output (same as -v 0)\n");
  printf("    -b samples     block size for scores (default 256; 0: sample by sample)\n");
  printf("    -j threads     threads to render with (default: one per core)\n");
  printf("    -r seed        seed for randomized noise and for notes' own random numbers\n");
  printf("                   (also SYD_SEED)\n");
  printf("    -x             allow score to override duration\n");
  printf("    -pN #          instrument parameter override (N may be 4-15)\n");
  exit(1);
}

char    gPatchFilename[256] = "";
char    gOutputFilename[256] = "";
ssfloat gDuration = 0;
int     gSampleRate = 0;
int     gVerbose = 1;
bool    gDurationOverride = false;
ssfloat gInstrumentParam[16] = {0,0,0,0,1.0,440.0,0,
                                0,0,0,0,0,0,0};

PatchOwner  *itsPatch;
SydFileSpec sydSpec;

int main(int argc, char **argv)
{  
  gThreads = std::thread::hardware_concurrency();
  if (gThreads < 1)
    gThreads = 1;
  if (getenv("SYD_SEED"))
    gFixedSeed = atoi(getenv("SYD_SEED"));
  LogMessage("%s version %s, by %s\n", SYD_TITLE, SYD_VERSION, SYD_AUTHOR);
  for (int i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1]) {
      case 'o':
        ++i;
        strcpy(gOutputFilename,argv[i]);
        break;
      case 'd': 
        ++i;
        gDuration = atof(argv[i]);
        break;
      case 's': 
        ++i;
        gSampleRate = atoi(argv[i]);
        break;
      case 'v':
        ++i;
        gVerbose = atoi(argv[i]);
        break;
      case 'q':
        gVerbose = 0;
        break;
      case 'b':
        ++i;
        gBlockSize = atoi(argv[i]);
        break;
      case 'j':
        ++i;
        gThreads = atoi(argv[i]);
        break;
      case 'r':
        ++i;
        gFixedSeed = atoi(argv[i]);
        break;
      case 'x':
        gDurationOverride = true;
        break;
      case 'p':
        if (isdigit(argv[i][2])) {
          int   n = atoi(&argv[i][2]);
          ++i;
          double v = atof(argv[i]);
          gInstrumentParam[n] = v;
        }
        break;
      default:
        printf("Unrecognized parameter: %s\n\n", argv[i]);
        SyntaxExit();
      }
    }
    else if (gPatchFilename[0] == 0)
    {
       strcpy(gPatchFilename,argv[i]);
    }
    else {
        SyntaxExit();
    }
  }
  if (gPatchFilename[0] == 0)
    SyntaxExit();

  strcpy(sydSpec.name, gPatchFilename);

  // Create patch   
  itsPatch = new PatchOwner();

  LogMessage("Reading Patch\n");

  // Load it in (overriding outspec if necessary)
  itsPatch->mainInst->OpenSpec(&sydSpec);

  // Synthesize it
  LogMessage("Beginning Synthesis\n");
  itsPatch->PerformSynthesis();
  // !!! DO THE FOLLOWING IN A THREAD - find a way to interrupt via key here...
  while (itsPatch->sampleNbr < itsPatch->nbrSamples)
  {
    if (gVerbose) {
      if (itsPatch->sampleNbr)
        LogMessage("%.2f%%\n", itsPatch->sampleNbr*100.0/itsPatch->nbrSamples);
    }
    itsPatch->SynthIdle();
  }
  if (gVerbose)
    putc('\n', stdout);  
  // Cleanup 
  if (itsPatch->sampleNbr < itsPatch->nbrSamples)
    itsPatch->CleanUpSynthesis();

  return 0;
}

