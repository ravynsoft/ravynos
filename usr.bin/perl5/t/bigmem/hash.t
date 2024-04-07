#!perl
BEGIN {
    chdir 't' if -d 't';
    @INC = "../lib";
    require './test.pl';
}

use Config qw(%Config);

$ENV{PERL_TEST_MEMORY} >= 4
    or skip_all("Need ~4Gb for this test");
$Config{ptrsize} >= 8
    or skip_all("Need 64-bit pointers for this test");

plan(2);

sub exn {
    my ($code_string) = @_;
    local $@;
    return undef if eval "do { $code_string }; 1";
    return $@;
}

like(exn('my $h = { "x" x 2**31, undef }'),
     qr/^\QSorry, hash keys must be smaller than 2**31 bytes\E\b/,
     "hash constructed with huge key");

TODO: {
    local $TODO = "Doesn't yet work with OP_MULTIDEREF";
    like(exn('my %h; %h{ "x" x 2**31 } = undef'),
         qr/^\QSorry, hash keys must be smaller than 2**31 bytes\E\b/,
         "assign to huge hash key");
}
