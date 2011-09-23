Version 1.0.3  5-8-2009

* Fixed a thread-related bug which was causing bursts of static in wave files.

Version 1.0.2 2-12-2007

* Reduced node size in patch panel from 64 to 40.
* Turned off "auto connect" feature by default.  Use the "Patch Options" menu to turn it back on.


JSyd ReadMe
-----------
JSyd is a Java port of the software synthesize Syd, and is backward
compatible with patches produced with Syd.  

On Macintosh and Windows systems, you can start JSyd by double-clicking
on the file JSyd.jar.

You can start JSyd on the command line by typing

java -cp JSyd.jar JSydApp

You can find documentation and a tutorial in the doc/ folder.  This 
documentation is based on the original Syd documentation, and may 
contain some errors, due to changes in the implementation.

Known Issues

* If you edit two or more patches simultaneously that are both set up to output
to the same file (such as "untitled.wav") then you may experience redrawing
and playback problems, due to both patches writing and reading the same file.

* The Mandelbrot and GNoise functions are not yet implemented.

* The new "zap-link" or "auto connect" feature, which automatically links modules together when
you drag them, still needs some finessing.  




