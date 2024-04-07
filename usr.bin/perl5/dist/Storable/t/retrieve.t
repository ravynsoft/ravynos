#!./perl
#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  Copyright (c) 2017, cPanel Inc
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

sub BEGIN {
    unshift @INC, 'dist/Storable/t' if $ENV{PERL_CORE} and -d 'dist/Storable/t';
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
    require 'st-dump.pl';
}


use Storable qw(store retrieve nstore);
use Test::More tests => 20;

$a = 'toto';
$b = \$a;
$c = bless {}, CLASS;
$c->{attribute} = 'attrval';
%a = ('key', 'value', 1, 0, $a, $b, 'cvar', \$c);
@a = ('first', '', undef, 3, -4, -3.14159, 456, 4.5,
	$b, \$a, $a, $c, \$c, \%a);

isnt(store(\@a, "store$$"), undef);
is(Storable::last_op_in_netorder(), '');
isnt(nstore(\@a, 'nstore'), undef);
is(Storable::last_op_in_netorder(), 1);
is(Storable::last_op_in_netorder(), 1);

$root = retrieve("store$$");
isnt($root, undef);
is(Storable::last_op_in_netorder(), '');

$nroot = retrieve('nstore');
isnt($root, undef);
is(Storable::last_op_in_netorder(), 1);

$d1 = &dump($root);
isnt($d1, undef);
$d2 = &dump($nroot);
isnt($d2, undef);

is($d1, $d2);

# Make sure empty string is defined at retrieval time
isnt($root->[1], undef);
is(length $root->[1], 0);

# $Storable::DEBUGME = 1;
{
    # len>I32: todo patch the storable image number into the strings, fake 2.10
    # $Storable::BIN_MINOR
    my $retrieve_blessed = "\x04\x0a\x08\x31\x32\x33\x34\x35\x36\x37\x38\x04\x08\x08\x08\x11\xff\x49\x6e\x74\xff\x72\x6e\x61\x6c\x73\x02\x00\x00\x00\x00";
    my $x = eval { Storable::mretrieve($retrieve_blessed); };
    # Long integer or Double size or Byte order is not compatible
    like($@, qr/^(Corrupted classname length|.* is not compatible|panic: malloc)/, "RT #130635 $@");
    is($x, undef, 'and undef result');
}

{
    # len>I32
    my $retrieve_hook = "\x04\x0a\x08\x31\x32\x33\x34\x35\x36\x37\x38\x04\x08\x08\x08\x13\x04\x49\xfe\xf4\xff\x72\x6e\x61\x6c\x73\x02\x00\x00\x00\x00";
    my $x = eval { Storable::mretrieve($retrieve_hook); };
    like($@, qr/^(Corrupted classname length|.* is not compatible|panic: malloc)/, "$@");
    is($x, undef, 'and undef result');
}

SKIP:
{
    # this can allocate a lot of memory, only do that if the testers tells us we can
    # the test allocates 2GB, but other memory is allocated too, so we want
    # at least 3
    $ENV{PERL_TEST_MEMORY} && $ENV{PERL_TEST_MEMORY} >= 3
      or skip "over 2GB memory needed for this test", 2;
    # len<I32, len>127: stack overflow
    my $retrieve_hook = "\x04\x0a\x08\x31\x32\x33\x34\x35\x36\x37\x38\x04\x08\x08\x08\x13\x04\x49\xfe\xf4\x7f\x72\x6e\x61\x6c\x73\x02\x00\x00\x00\x00";
    my $x = eval { Storable::mretrieve($retrieve_hook); };
    is($?, 0, "no stack overflow in retrieve_hook()");
    is($x, undef, 'either out of mem or normal error (malloc 2GB)');
}

END { 1 while unlink("store$$", 'nstore') }
