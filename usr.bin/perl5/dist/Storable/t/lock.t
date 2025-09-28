#!./perl
#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }

    require 'st-dump.pl';
}

use Test::More;
use Storable qw(lock_store lock_retrieve);

unless (&Storable::CAN_FLOCK) {
    plan(skip_all => "fcntl/flock emulation broken on this platform");
}

plan(tests => 5);

@a = ('first', undef, 3, -4, -3.14159, 456, 4.5);

#
# We're just ensuring things work, we're not validating locking.
#

isnt(lock_store(\@a, "store$$"), undef);
my $dumped = &dump(\@a);
isnt($dumped, undef);

$root = lock_retrieve("store$$");
is(ref $root, 'ARRAY');
is(scalar @a, scalar @$root);
is(&dump($root), $dumped);

END { 1 while unlink "store$$" }

