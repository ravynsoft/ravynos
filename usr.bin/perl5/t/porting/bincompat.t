#!./perl -w
BEGIN {
    chdir ".." if -e "./test.pl";
    push @INC, "lib";
}
use strict;
require './t/test.pl';
skip_all("Sorting order differs under EBCDIC") if $::IS_EBCDIC || $::IS_EBCDIC;

use Config;

my %legacy_different = (
    # define                       # string
    'VMS_WE_ARE_CASE_SENSITIVE' => 'VMS_SYMBOL_CASE_AS_IS',
    'WIN32_NO_REGISTRY'         => 'USE_NO_REGISTRY',
);

# As we need to call it direct, we'll take advantage of its result ordering:
my @to_check = qw(bincompat_options non_bincompat_options);
my @file = qw(perl.h perl.c);
my @var = qw(PL_bincompat_options non_bincompat_options);
my @V = map {s/^ //r} Internals::V();

while (my ($index, $sub) = each @to_check) {
    my $got = join ' ', sort &{Config->can($sub)}();
    is($got, $V[$index], "C source code has $sub in sorted order");
    open my $fh, "<", $file[$index]
        or die "Failed to open '$file[$index]': $!";
    my @strs;
    my @define;
    while (<$fh>) {
        if (/$var[$index]\[\]\s*=/ .. /^\s*"";/) {
            if (/ifdef\s+(\w+)/) {
                my $name = $1;
                # ignore PERL_HASH_ vars as they are handled differently
                # from the rest.
                $name=~/PERL_HASH_/ and next;
                push @define, $name;
            }
            elsif (/" ([^"]+)"/) {
                my $name = $1;
                # ignore PERL_HASH_ vars as they are handled differently
                # from the rest.
                $name=~/PERL_HASH_/ and next;
                push @strs, $name;
            }
        }
    }
    foreach my $j (0 .. $#strs) {
        my $want = $legacy_different{$define[$j]} || $define[$j];
        my $str = $strs[$j];
        is($strs[$j],$want, "String and define $j are the same ($strs[$j]) for $var[$index] in $file[$index]");
    }
    my @sorted_strs = sort @strs;
    is("@strs","@sorted_strs", "Strings are sorted for $var[$index] in $file[$index]");
}

done_testing();
