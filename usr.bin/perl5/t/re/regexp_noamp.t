#!./perl

# Doesn't look at the expect field if it contains $&.

$skip_amp = 1;
for $file ('./re/regexp.t', './t/re/regexp.t', ':re:regexp.t') {
  if (-r $file) {
    do $file or die $@;
    exit;
  }
}
die "Cannot find ./re/regexp.t or ./t/re/regexp.t\n";
