#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    @INC = qw(lib ../lib);
    plan(tests => 14);
}

use strict;
use warnings;

use mypragma (); # don't enable this pragma yet

BEGIN {
   is($^H{mypragma}, undef, "Shouldn't be in %^H yet");
}

is(mypragma::in_effect(), undef, "pragma not in effect yet");
{
    is(mypragma::in_effect(), undef, "pragma not in effect yet");
    eval qq{is(mypragma::in_effect(), undef, "pragma not in effect yet"); 1}
	or die $@;

    use mypragma;
    use Sans_mypragma;
    is(mypragma::in_effect(), 42, "pragma is in effect within this block");
    is(Sans_mypragma::affected(), undef,
	"pragma not in effect outside this file");
    eval qq{is(mypragma::in_effect(), 42,
	       "pragma is in effect within this eval"); 1} or die $@;

    {
      no mypragma;
      is(mypragma::in_effect(), 0, "pragma no longer in effect");
      eval qq{is(mypragma::in_effect(), 0, "pragma no longer in effect"); 1}
	or die $@;
    }

    is(mypragma::in_effect(), 42, "pragma is in effect within this block");
    eval qq{is(mypragma::in_effect(), 42,
	       "pragma is in effect within this eval"); 1} or die $@;
}
is(mypragma::in_effect(), undef, "pragma no longer in effect");
eval qq{is(mypragma::in_effect(), undef, "pragma not in effect"); 1} or die $@;


BEGIN {
   is($^H{mypragma}, undef, "Should no longer be in %^H");
}
