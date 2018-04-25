#!/usr/bin/env perl
#

@list=@ARGV;
@ARGV=();

print << "EOM";
#include "cez_prayer_misc.h"
#include "cez_prayer_template_structs.h"

EOM

foreach $name (@list) {
print << "EOM";
extern struct template_map template_map_${name}[];
extern unsigned long template_map_${name}_count;
EOM
}

print << "EOM";

struct template_map_index template_map_index[] = {
EOM

foreach $name (@list) {
  print << "EOM";
  { "$name",  &template_map_${name}[0], &template_map_${name}_count },
EOM
}

print << "EOM";
  { NIL, NIL, NIL }
};
EOM
