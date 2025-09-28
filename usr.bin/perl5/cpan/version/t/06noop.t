#! /usr/local/perl -w
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More qw/no_plan/;

BEGIN {
    use_ok('version', 0.9929);
}

my $v1 = 'version'->new('1.2');
eval {$v1 = $v1 + 1};
like $@, qr/operation not supported with version object/, 'No math ops with version objects';
eval {$v1 = $v1 - 1};
like $@, qr/operation not supported with version object/, 'No math ops with version objects';
eval {$v1 = $v1 / 1};
like $@, qr/operation not supported with version object/, 'No math ops with version objects';
eval {$v1 = $v1 * 1};
like $@, qr/operation not supported with version object/, 'No math ops with version objects';
eval {$v1 = abs($v1)};
like $@, qr/operation not supported with version object/, 'No math ops with version objects';

eval {$v1 += 1};
like $@, qr/operation not supported with version object/, 'No math ops with version objects';
eval {$v1 -= 1};
like $@, qr/operation not supported with version object/, 'No math ops with version objects';
eval {$v1 /= 1};
like $@, qr/operation not supported with version object/, 'No math ops with version objects';
eval {$v1 *= 1};
like $@, qr/operation not supported with version object/, 'No math ops with version objects';
