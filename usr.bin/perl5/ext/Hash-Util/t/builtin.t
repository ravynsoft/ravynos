#!/usr/bin/perl -Tw

use strict;
use Test::More;

my @Exported_Funcs;
BEGIN {
    @Exported_Funcs = qw( bucket_ratio num_buckets used_buckets );
    plan tests => 13 + @Exported_Funcs;
    use_ok 'Hash::Util', @Exported_Funcs;
}
foreach my $func (@Exported_Funcs) {
    can_ok __PACKAGE__, $func;
}

my %hash;

is(bucket_ratio(%hash), 0, "Empty hash has no bucket_ratio");
is(num_buckets(%hash), 8, "Empty hash should have eight buckets");
is(used_buckets(%hash), 0, "Empty hash should have no used buckets");

$hash{1}= 1;
is(bucket_ratio(%hash), "1/8", "hash has expected bucket_ratio");
is(num_buckets(%hash), 8, "hash should have eight buckets");
is(used_buckets(%hash), 1, "hash should have one used buckets");

$hash{$_}= $_ for 2..7;

like(bucket_ratio(%hash), qr!/(?:8|16)!, "hash has expected number of buckets in bucket_ratio");
my $num= num_buckets(%hash);
ok(($num == 8 || $num == 16), "hash should have 8 or 16 buckets");
cmp_ok(used_buckets(%hash), "<", 8, "hash should have one used buckets");

$hash{8}= 8;
like(bucket_ratio(%hash), qr!/(?:8|16)!, "hash has expected number of buckets in bucket_ratio");
$num= num_buckets(%hash);
ok(($num == 8 || $num == 16), "hash should have 8 or 16 buckets");
cmp_ok(used_buckets(%hash), "<=", 8, "hash should have at most 8 used buckets");


