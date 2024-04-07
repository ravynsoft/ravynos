#!perl

use strict;
use warnings;

use Test::More tests => 7;

use_ok('XS::APItest');

# test sv_rvweaken() and sv_get_backrefs()
#
# weaken() maps to sv_rvweaken() and is the same as the one
# from Scalar::Utils - we recreate it in XS::APItest so
# we can test it even if we build without Scalar::Utils
#
# has_backrefs() maps to sv_get_backrefs(), which would not
# normally be useful to Perl code. (Er, maybe :-)

# has_backrefs is really an internal routine
# which would not normally have to worry about refs
# and things like that, but to use it from perl we cant
# have an AV/HV without having an RV wrapping it, so we
# mandate the ref always.

my $foo= "foo";
my $bar= "bar";

my $scalar_ref= \$foo;
my $array_ref= [ qw(this is an array) ];
my $hash_ref= { this => is => a => 'hash' };

my $nrml_scalar_ref= \$bar;
my $nrml_array_ref= [ qw( this is an array ) ];
my $nrml_hash_ref= { this => is => a => 'hash' };

# we could probably do other tests here, such as
# verify the refcount of the referents, but maybe
# another day.
apitest_weaken(my $weak_scalar_ref= $scalar_ref);
apitest_weaken(my $weak_array_ref= $array_ref);
apitest_weaken(my $weak_hash_ref= $hash_ref);

ok(has_backrefs($scalar_ref), "scalar with backrefs");
ok(has_backrefs($array_ref), "array with backrefs");
ok(has_backrefs($hash_ref), "hash with backrefs");

ok(!has_backrefs($nrml_scalar_ref), "scalar without backrefs");
ok(!has_backrefs($nrml_array_ref), "array without backrefs");
ok(!has_backrefs($nrml_hash_ref), "hash without backrefs");

1;

