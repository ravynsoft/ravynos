package OtherTypes;
no warnings;

our $foo = 23;
our @foo = "bar";
our %foo = (mouse => "trap");
open foo, "<", $0;

format foo =
foo
.

BEGIN {
    $main::pvio = *foo{IO};
    $main::pvfm = *foo{FORMAT};
}

sub foo { 1 }

use autodie 'foo';

1;
