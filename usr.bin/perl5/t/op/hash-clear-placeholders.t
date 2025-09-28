#!/usr/bin/perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( '../lib' );
    skip_all_without_dynamic_extension("Devel::Peek");
    skip_all_without_dynamic_extension("Hash::Util");
}

use strict;
use Devel::Peek;
use Hash::Util qw(lock_keys_plus);

my %hash = (chr 255 => 42, chr 256 => 6 * 9);
lock_keys_plus(%hash, "baz");

my $tempfile = tempfile();

local *OLDERR;
open(OLDERR, ">&STDERR") || die "Can't dup STDERR: $!";
open(STDERR, ">", $tempfile) ||
    die "Could not open '$tempfile' for write: $^E";

my $sep = "=-=-=-=\n";
Dump \%hash;
print STDERR $sep;
delete $hash{chr 255};
Internals::hv_clear_placeholders(%hash);
Dump \%hash;
print STDERR $sep;
delete $hash{chr 256};
Internals::hv_clear_placeholders(%hash);
Dump \%hash;

open(STDERR, ">&OLDERR") || die "Can't dup OLDERR: $!";
open(my $fh, "<", $tempfile) ||
    die "Could not open '$tempfile' for read: $^E";
local $/;
my $got = <$fh>;

my ($first, $second, $third) = split $sep, $got;

like($first, qr/\bPERL_MAGIC_rhash\b/, 'first dump has rhash magic');
like($second, qr/\bPERL_MAGIC_rhash\b/, 'second dump has rhash magic');
like($third, qr/\bPERL_MAGIC_rhash\b/, 'third dump has rhash magic');

like($first, qr/\bHASKFLAGS\b/, 'first dump has HASHKFLAGS set');
like($second, qr/\bHASKFLAGS\b/, 'second dump has HASHKFLAGS set');
unlike($third, qr/\bHASKFLAGS\b/, 'third dump has HASHKFLAGS clear');

like($first, qr/\bMG_LEN = 1\b/, 'first dump has 1 placeholder');
unlike($second, qr/\bMG_LEN\b/, 'second dump has 0 placeholders');
unlike($third, qr/\bMG_LEN\b/, 'third dump has 0 placeholders');

done_testing();
