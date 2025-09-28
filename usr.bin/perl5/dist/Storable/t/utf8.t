#!./perl -w
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
}

use strict;

use Storable qw(thaw freeze);
use Test::More tests => 6;

my $x = chr(1234);
is($x, ${thaw freeze \$x});

# Long scalar
$x = join '', map {chr $_} (0..1023);
is($x, ${thaw freeze \$x});

# Char in the range 127-255 (probably) in utf8.  This just won't work for
# EBCDIC for early Perls.
$x = ($] lt 5.007_003) ? chr(175) : chr(utf8::unicode_to_native(175))
   . chr (256);
chop $x;
is($x, ${thaw freeze \$x});

# Storable needs to cope if a frozen string happens to be internal utf8
# encoded

$x = chr 256;
my $data = freeze \$x;
is($x, ${thaw $data});

$data .= chr 256;
chop $data;
is($x, ${thaw $data});


$data .= chr 256;
# This definitely isn't valid
eval {thaw $data};
like($@, qr/corrupt.*characters outside/);
