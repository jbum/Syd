#!/usr/bin/perl

while (<patches/*.syd>)
{
  $fname = $_;
  open IFILE, "$fname" or die "Can't open $fname\n";
  my %mods = ();
  my %funcs = ();
  while (<IFILE>)
  {
    if (/MOD \S+\s(\S+)/) {
      next if $1 eq 'out';
      $mods{$1} = 1;
    }
    while (/(\w+)\(/) {
      $funcs{$1} = 1;
      s/\w+\(/ \(/;
    }
  }
  close IFILE;
  my $modList = join ',', sort keys %mods;
  my $funcList = join ',', sort keys %funcs;
  my ($name) = $fname =~ /patches\/(.*)/;
  printf "%-16s: $modList\n", $name;
  print "  $funcList\n" if $funcList;
}