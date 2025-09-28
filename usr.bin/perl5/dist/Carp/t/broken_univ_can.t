use strict;
use warnings;

# [perl #132910]
# This mock-up breaks Test::More.  Don’t use Test::More.

{
    no warnings 'redefine';
    sub UNIVERSAL::can { die; }
}

# Carp depends on this to detect the override:
BEGIN { no warnings 'portable'; $UNIVERSAL::can::VERSION = 0xbaff1ed_bee; }

use Carp;

eval {
    sub { confess-sins }->(bless[], 'Foo');
};
print "1..1\n";
if ($@ !~ qr/^-sins at /) {
  print "not ok 1\n";
  print "# Expected -sins at blah blah blah...\n";
  print "# Instead, we got:\n";
  $@ =~ s/^/#   /mg;
  print $@;
}
else {
  print "ok 1\n";
}
