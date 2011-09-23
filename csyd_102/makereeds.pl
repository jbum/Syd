#!/usr/bin/perl

@intervals = (0,2,4,7,9);

foreach $i (0..25)
{
  $nam = 60+$i;
  # $pitch = 60+$i;
  $pitch = (36+12)+12*int($i/5)+$intervals[$i%5];
  $cmd = "csyd -o reed_$nam.wav patches/plucktone.syd -p4 .05 -p5 $pitch -d 2";
  print `$cmd`;
}