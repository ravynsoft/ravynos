#!./perl

# Matches regular expressions against strings with no terminating null
# character.

print("1..0 # Skip No XS::APItest under miniperl\n"), exit 0 if
  !defined &DynaLoader::boot_DynaLoader;

$no_null = 1;
for $file ('./re/regexp.t', './t/re/regexp.t', ':re:regexp.t') {
  if (-r $file) {
    do $file or die $@;
    exit;
  }
}
die "Cannot find ./re/regexp.t or ./t/re/regexp.t\n";
