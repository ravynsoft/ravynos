#!perl

use strict;
use warnings;

use Data::Dumper;
use Test::More tests => 10;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );


my $hash = { foo => 42 };

for my $useperl (0..1) {
    my $dumper = Data::Dumper->new([$hash]);
    $dumper->Terse(1);
    $dumper->Indent(2);
    $dumper->Useperl($useperl);

    is $dumper->Dump, <<'WANT', "Terse(1), Indent(2), Useperl($useperl)";
{
  'foo' => 42
}
WANT
}

my $dumper;

$dumper = Data::Dumper->new([$hash]);
my $dumpstr_noterse = _dumptostr($dumper);

$dumper = Data::Dumper->new([$hash]);
$dumper->Terse();
is _dumptostr($dumper), $dumpstr_noterse;

$dumper = Data::Dumper->new([$hash]);
$dumper->Terse(0);
is _dumptostr($dumper), $dumpstr_noterse;

$dumper = Data::Dumper->new([$hash]);
$dumper->Terse(1);
isnt _dumptostr($dumper), $dumpstr_noterse;

$dumper = Data::Dumper->new([$hash]);
is $dumper->Terse(1), $dumper;
is $dumper->Terse, 1;
is $dumper->Terse(undef), $dumper;
is $dumper->Terse, undef;
is _dumptostr($dumper), $dumpstr_noterse;
