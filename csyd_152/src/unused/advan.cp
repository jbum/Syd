/*



Cool Features
=============
1. Graphical interface makes patching easy to understand.

2. Score Generation is integrated with instruments.  For example,
scores can use instruments which in turn use scores.  Allows
for the "patch" system to be used to create more complex scores.
A "movement" can be the utterence of a note, which in turn consists of
a score.

3. Scores can output into amplifier and reverb making last minute 
adjustments easier.

4. Random Scores can be used to multiplex notes, create reverb effects.

5. Random scores can be used to convert parameters in a score for use
with an instrument which expects different parameters.

6. Easy to define function table based on mathematical formula.




OVERALL GOALS 8/25/97
=====================
1. Simple to use - patch osc to speaker, hear a sound.
2. Avoid unnecessary complexity (e.g. "merge" "kamp" vs "amp")
3. Nearly as flexible as CSound
4. Midi, Java, Fractals

My goal is to create an acoustic compiler with the power of CSound or
cmusic, and the ease of use of a patchcord synthesizer.  You should
be able to produce sound in a few seconds by patching an oscillator
to a speaker. 

The main thing preventing serious computer music tools from reaching a
wider audience of composers is unnecessary complexity due to historical 
precedent.  CSound is really only easily understood if you are knowledgeable
enough to have programmed it.

To date, acoustic compiler writers have not been good interface designers.

The Big Enchilada: I'm assuming that given the current increases in CPU speed,
we should expect to be able to do real-time synthesis in java in 3-4 years.
Once this is doable, Java would make an ideal platform for this program because
new sound modules could be shared easily over the net, and it would be
multiplatform.

* Loudness is 0-1   dB must be converted if used.
* Frequency is Hz
* Time is seconds


*/