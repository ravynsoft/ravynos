#!./perl

use strict;
use warnings;

use Config;
use Storable qw(dclone);
use Test::More;

BEGIN {
    plan skip_all => 'Storable was not built'
        if $ENV{PERL_CORE} && $Config{'extensions'} !~ /\b Storable \b/x;
    plan skip_all => 'Need 64-bit pointers for this test'
        if $Config{ptrsize} < 8 and $] > 5.013;
    plan skip_all => 'Need 64-bit int for this test on older versions'
        if $Config{uvsize} < 8 and $] < 5.013;
    plan skip_all => 'Need ~4 GiB memory for this test, set PERL_TEST_MEMORY > 4'
        if !$ENV{PERL_TEST_MEMORY} || $ENV{PERL_TEST_MEMORY} < 4;
}

# Just too big to fit in an I32.
my $huge = int(2 ** 31);
# v5.24.1c/v5.25.1c switched to die earlier with "Too many elements",
# which is much safer.
my $has_too_many = ($Config{usecperl} and
      (($] >= 5.024001 and $] < 5.025000)
       or $] >= 5.025001)) ? 1 : 0;

# These overlarge sizes are enabled only since Storable 3.00 and some
# cases need cperl support. Perl5 (as of 5.24) has some internal
# problems with >I32 sizes, which only cperl has fixed.
# perl5 is not yet 2GB safe, esp. with hashes.

# string len (xpv_cur): STRLEN (ptrsize>=8)
# array size (xav_max): SSize_t (I32/I64) (ptrsize>=8)
# hash size (xhv_keys):
#    IV            - 5.12   (ivsize>=8)
#    STRLEN  5.14  - 5.24   (size_t: U32/U64)
#    SSize_t 5.22c - 5.24c  (I32/I64)
#    U32     5.25c -
# hash key: I32

my @cases = (
    ['huge string',
     sub { my $s = 'x' x $huge; \$s }],

    ['array with huge element',
     sub { my $s = 'x' x $huge; [$s] }],

    ['hash with huge value',
     sub { my $s = 'x' x $huge; +{ foo => $s } }],

    # There's no huge key, limited to I32.
  ) if $Config{ptrsize} > 4;


# An array with a huge number of elements requires several gigabytes of
# virtual memory. On darwin it is evtl killed.
if ($Config{ptrsize} > 4 and !$has_too_many) {
    # needs 20-55G virtual memory, 4.6M heap and several minutes on a fast machine 
    if ($ENV{PERL_TEST_MEMORY} >= 55) {
        push @cases,
          [ 'huge array',
            sub { my @x; $x[$huge] = undef; \@x } ];
    } else {
        diag "skip huge array, need PERL_TEST_MEMORY >= 55";
    }
}

# A hash with a huge number of keys would require tens of gigabytes of
# memory, which doesn't seem like a good idea even for this test file.
# Unfortunately even older 32bit perls do allow this.
if (!$has_too_many) {
    # needs >90G virtual mem, and is evtl. killed
    if ($ENV{PERL_TEST_MEMORY} >= 96) {
        # number of keys >I32. impossible to handle with perl5, but Storable can.
        push @cases,
          ['huge hash',
           sub { my %x = (0 .. $huge); \%x } ];
    } else {
        diag "skip huge hash, need PERL_TEST_MEMORY >= 96";
    }
}


plan tests => 2 * scalar @cases;

for (@cases) {
    my ($desc, $build) = @$_;
    diag "building test input: $desc";
    my ($input, $exn, $clone);
    diag "these huge subtests need a lot of memory and time!" if $desc eq 'huge array';
    $input = $build->();
    diag "running test: $desc";
    $exn = $@ if !eval { $clone = dclone($input); 1 };

    is($exn, undef, "$desc no exception");
    is_deeply($input, $clone, "$desc cloned");
    #ok($clone, "$desc cloned");

    # Ensure the huge objects are freed right now:
    undef $input;
    undef $clone;
}
