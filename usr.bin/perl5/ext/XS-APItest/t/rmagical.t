#!perl

# Consider two kinds of magic :
# A : PERL_MAGIC_uvar, with get (but no set) magic
# B : PERL_MAGIC_ext, with a zero vtbl
# If those magic are attached on a sv in such a way that the MAGIC chain
# looks like sv -> B -> A -> NULL (i.e. we first apply A and then B), then
# mg_magical won't turn SvRMAGICAL on. However, if the chain is in the
# opposite order (sv -> A -> B -> NULL), SvRMAGICAL used to be turned on.

use strict;
use warnings;

use Test::More tests => 3;

use_ok('XS::APItest');

my (%h1, %h2);
my @f;

rmagical_cast(\%h1, 0); # A
rmagical_cast(\%h1, 1); # B
@f = rmagical_flags(\%h1);
ok(!$f[2], "For sv -> B -> A -> NULL, SvRMAGICAL(sv) is false");

rmagical_cast(\%h2, 1); # B
rmagical_cast(\%h2, 0); # A
@f = rmagical_flags(\%h2);
ok(!$f[2], "For sv -> A -> B -> NULL, SvRMAGICAL(sv) is false");
