#!perl

# Test that exception catching is set up early enough when executing
# pp_entereval() etc. There used to be a gap where an exception could
# be raised before perl was ready to catch it.
#
# RT #105930: eval 'UNITCHECK{die}' crashes inside FETCH

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use warnings;
use strict;

plan 12;

{
    package EvalOnFetch;
    sub TIESCALAR { bless \(my $z = $_[1]), $_[0] }
    sub FETCH { eval ${$_[0]} // "died" }
}

tie my $begindie, "EvalOnFetch", "BEGIN { die } 123";
is "$begindie", "died";
tie my $unitcheckdie, "EvalOnFetch", "UNITCHECK { die } 123";
is "$unitcheckdie", "died";
tie my $rundie, "EvalOnFetch", "die; 123";
is "$rundie", "died";
tie my $runok, "EvalOnFetch", "123";
is "$runok", 123;

eval { undef };
is eval "BEGIN { die } 123", undef;
is eval "UNITCHECK { die } 123", undef;
is eval "die; 123", undef;
is eval "123", 123;

{
    package TryOnFetch;
    sub TIESCALAR { bless \(my $z = $_[1]), $_[0] }
    sub FETCH { eval { ${$_[0]} ? die : undef; 123 } // "died" }
}

tie my $trydie, "TryOnFetch", 1;
is "$trydie", "died";
tie my $tryok, "TryOnFetch", 0;
is "$tryok", 123;

eval { undef };
is do { eval { die; 123 } }, undef;
is do { eval { undef; 123 } }, 123;

1;
