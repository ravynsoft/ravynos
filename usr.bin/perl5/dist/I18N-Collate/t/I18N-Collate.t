#!./perl

# at least in the CPAN version we're sometimes called with -w, for example
# during 'make test', so disable them explicitly and only turn them on again for
# the deprecation test.
use strict;
no warnings;

BEGIN {
    require Config; import Config;
    if (!$::Config{d_setlocale} || $::Config{ccflags} =~ /\bD?NO_LOCALE\b/) {
	print "1..0\n";
	exit;
    }
}

use Test::More tests => 7;

BEGIN {use_ok('I18N::Collate');}

$a = I18N::Collate->new("foo");

isa_ok($a, 'I18N::Collate');

{
    use warnings;
    local $SIG{__WARN__} = sub { $@ = $_[0] };
    $b = I18N::Collate->new("foo");
    like($@, qr/\bHAS BEEN DEPRECATED\b/);
    $@ = '';
}

is($a, $b, 'same object');

$b = I18N::Collate->new("bar");
unlike($@, qr/\bHAS BEEN DEPRECATED\b/);

isnt($a, $b, 'different object');

cmp_ok($a lt $b, '!=', $a gt $b);
