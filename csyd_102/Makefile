IDIR =src
ODIR=obj
SDIR=src

CC=g++
CFLAGS=-I$(IDIR) -I./STK/include -g -m32 -Wno-write-strings

# use this if necessary
# CFLAGS += -DIS_BIGENDIAN
#

LDIR =../lib

LIBS=-lm

_DEPS = ss.h ssmodule.h patchowner.h modlist.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS)) sydlocal.h

_OBJ =  ssadsr.o ssamplifier.o ssbutter.o sscscore.o ssdelay.o \
        ssexpression.o ssfilter.o ssfinput.o ssfolder.o ssftable.o \
        ssgassign.o sshammeractuator.o sshammerbank.o ssinverter.o \
        ssmaraca.o ssmixer.o ssmodule.o ssnoise.o ssoscillator.o  \
        ssoutput.o sspinput.o sspluck.o ssrandscore.o sssample.o \
        sssamplefile.o ssscore.o sssmooth.o ssstorage.o ssstorefile.o \
        ssstoremem.o ssthreshhold.o ssutils.o ssskiniscore.o \
        patchowner.o modlist.o expmgr.o expfuncs.o wavetables.o \
        ssmain.o
        

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

csyd: $(OBJ)
	g++ -O3 -o $@ $^ $(CFLAGS) $(LIBS)

csydd: $(OBJ)
	g++ -g -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
