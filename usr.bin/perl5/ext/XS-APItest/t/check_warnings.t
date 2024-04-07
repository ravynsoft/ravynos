#!perl

# This test checks to make sure that a BEGIN block created from an XS call
# does not implicitly change the current warning scope, causing a CHECK
# or INIT block created after the corresponding phase to warn when it
# shouldn’t.

use Test::More tests => 1;

$SIG{__WARN__} = sub { $w .= shift };

use warnings;
eval q|
  BEGIN{
    no warnings;
    package XS::APItest; require XSLoader; XSLoader::load()
  }
|;

is $w, undef, 'No warnings about CHECK and INIT in warningless scope';
