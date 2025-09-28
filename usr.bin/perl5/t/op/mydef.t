#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

eval 'my $_';
like $@, qr/^Can't use global \$_ in "my" at /;

{
    # using utf8 allows $_ to be declared with 'my'
    # GH #18449
    use utf8;
    eval 'my $_;';
    like $@, qr/^Can't use global \$_ in "my" at /;
}

done_testing;
