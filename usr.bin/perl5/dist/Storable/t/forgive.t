#!./perl
#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#
# Original Author: Ulrich Pfeifer
# (C) Copyright 1997, Universitat Dortmund, all rights reserved.
#

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

use Storable qw(store retrieve);
use Test::More;

# problems with 5.00404 when in an BEGIN block, so this is defined here
if (!eval { require File::Spec; 1 } || $File::Spec::VERSION < 0.8) {
    plan(skip_all => "File::Spec 0.8 needed");
    # Mention $File::Spec::VERSION again, as 5.00503's harness seems to have
    # warnings on.
    exit $File::Spec::VERSION;
}

plan(tests => 8);

*GLOB = *GLOB; # peacify -w
my $bad = ['foo', \*GLOB,  'bar'];
my $result;

eval {$result = store ($bad , "store$$")};
is($result, undef);
isnt($@, '');

$Storable::forgive_me=1;

my $devnull = File::Spec->devnull;

open(SAVEERR, ">&STDERR");
open(STDERR, '>', $devnull) or 
  ( print SAVEERR "Unable to redirect STDERR: $!\n" and exit(1) );

eval {$result = store ($bad , "store$$")};

open(STDERR, ">&SAVEERR");

isnt($result, undef);
is($@, '');

my $ret = retrieve("store$$");
isnt($ret, undef);
is($ret->[0], 'foo');
is($ret->[2], 'bar');
is(ref $ret->[1], 'SCALAR');


END { 1 while unlink "store$$" }
