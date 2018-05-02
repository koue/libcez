#!/usr/bin/env perl
#

$prefix = shift(@ARGV);
$count = 0;

@templates = ();
foreach $i (@ARGV) {
  if  ($i =~ /(\S+)\.t/) {
    push(@templates, $1);
  } else {
    push(@templates, $i);
  }
  $count++;
}
@ARGV = ();
@templates = sort(@templates);

print <<'EOM';
#include "cez_prayer_misc.h"
#include "cez_prayer_template_structs.h"
EOM

foreach $i (@templates) {
  print "struct template _template_${prefix}_${i};\n";
}

print <<"EOM";

struct template_map template_map_${prefix}[] = {
EOM

foreach $i (@templates) {
  print <<"EOM";
    { "${i}", &_template_${prefix}_${i} },
EOM
}

print <<"EOM";
    { NIL, NIL }
};

unsigned long template_map_${prefix}_count = ${count};
EOM
