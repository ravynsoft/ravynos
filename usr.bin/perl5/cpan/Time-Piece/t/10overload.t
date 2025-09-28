use strict;
use warnings;
use Time::Piece;
use Test::More;

eval 'use Math::BigInt';
plan skip_all => "Math::BigInt required for testing overloaded operands" if $@;

my $t = Time::Piece->gmtime(315532800); # 00:00:00 1/1/1980
isa_ok $t, 'Time::Piece';
is $t->cdate, 'Tue Jan  1 00:00:00 1980', 'got expected gmtime with int secs';

$t = Time::Piece->gmtime(Math::BigInt->new('315532800')); # 00:00:00 1/1/1980
is $t->cdate, 'Tue Jan  1 00:00:00 1980', 'got same time with overloaded secs';


my $big_hour = Math::BigInt->new('3600');
 
$t = $t + $big_hour;
is $t->cdate, 'Tue Jan  1 01:00:00 1980', 'add overloaded value';
 
$t = $t - $big_hour;
is $t->cdate, 'Tue Jan  1 00:00:00 1980', 'sub overloaded value';

done_testing;
