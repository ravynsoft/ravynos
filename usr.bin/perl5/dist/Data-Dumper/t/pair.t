#!./perl -w
#
# test for $Data::Dumper::Pair AKA Data::Dumper->new([ ... ])->Pair('...')
#

use strict;
use warnings;

our ($want_colon, $want_comma);
use Test::More tests => 9;

no warnings qw(once);

require_ok 'Data::Dumper';

my $HASH = { alpha => 'beta', gamma => 'vlissides' };
my $WANT = q({'alpha' => 'beta','gamma' => 'vlissides'});

$Data::Dumper::Useperl = 1;
$Data::Dumper::Indent = 0;
$Data::Dumper::Terse = 1;
$Data::Dumper::Sortkeys = 1;

$want_colon = $want_comma = $WANT;
$want_colon =~ s/=>/:/g;
$want_comma =~ s/ => /,/g;

####################### XS Tests #####################

SKIP: {
    skip 'XS extension not loaded', 3 unless (defined &Data::Dumper::Dumpxs);
    is (Data::Dumper::DumperX($HASH), $WANT, 
	'XS: Default hash key/value separator: " => "');
    local $Data::Dumper::Pair = ' : ';
    is (Data::Dumper::DumperX($HASH), $want_colon, 'XS: $Data::Dumper::Pair = " : "');
    my $dd = Data::Dumper->new([ $HASH ])->Pair(',');
    is ($dd->Dumpxs(), $want_comma, 
	'XS: Data::Dumper->new([ $HASH ])->Pair(",")->Dumpxs()');
};

###################### Perl Tests ####################

{
    is ($Data::Dumper::Pair, ' => ', 'Perl: $Data::Dumper::Pair eq " => "');
    is (Data::Dumper::Dumper($HASH), $WANT, 
	'Perl: Default hash key/value separator: " => "');
    local $Data::Dumper::Pair = ' : ';
    is (Data::Dumper::Dumper($HASH), $want_colon, 'Perl: $Data::Dumper::Pair = " : "');
    my $dd = Data::Dumper->new([ $HASH ])->Pair(',');
    is ($dd->Pair(), ',', 
	'Perl: Data::Dumper->new([ $HASH ])->Pair(",")->Pair() eq ","');
    is ($dd->Dump(), $want_comma, 'Perl: Data::Dumper->new([ $HASH ])->Pair(",")->Dump()');
}
