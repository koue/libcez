#!/usr/bin/env perl
#
# Generate proper list of dependancies between templates

%uses = ();

while ($file=shift(@ARGV)) {
  $file = $1 if ($file =~ /([\w-_]+)\.t/);

  open(FILE, "<${file}.t") or die "failed to open ${file}: $!\n";

  while (<FILE>) {
    next unless /^%\s+CALL ([\w-_]+)/;

    $uses{$file} = [] if (not $uses{$file});
    push(@{$uses{$file}}, $1);
  }

  close(FILE);
}

foreach $i (sort keys %uses) {
  # Sort and uniq
  @{$uses{$i}} = keys %{{ map { $_ => 1 } sort(@{$uses{$i}}) }};
}

foreach $i (sort keys %uses) {
  printf("%s.html: %s.t", $i, $i);
  foreach $j (@{$uses{$i}}) {
    @list = ();
    recurse($j, {}, \@list);
    foreach $k (@list) {
      printf(" %s.t", $k);
    }
  }
  printf("\n");
}

exit(0);

sub recurse {
  my ($i, $usedref, $listref) = @_;

  # Remove repeated references to any given template/
  return if defined($$usedref{$i});
  $$usedref{$i} = 1;

  push (@{$listref}, $i);
  foreach $j (@{$uses{$i}}) {
    recurse($j, $usedref, $listref);
  }
}
