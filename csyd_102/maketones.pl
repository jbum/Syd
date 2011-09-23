#!/usr/bin/perl

@intervals = (0,2,4,7,9);

foreach $i (0..25)
{
  $nam = 60+$i;
  # $pitch = 60+$i;
  $pi = 25-$i;
  $pitch = (36)+12*int($pi/5)+$intervals[$pi%5];
  $dur = .5 + (2*$i/25.0);
  $cmd = "csyd -o reed_$nam.wav patches/plucktone.syd -p4 .05 -p5 $pitch -d $dur";
  print `$cmd`;
}