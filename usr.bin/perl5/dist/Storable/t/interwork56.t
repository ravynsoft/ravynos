#!./perl -w
#
#  Copyright 2002, Larry Wall.
#
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

# I ought to keep this test easily backwards compatible to 5.004, so no
# qr//;

# This test checks whether the kludge to interwork with 5.6 Storables compiled
# on Unix systems with IV as long long works.

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
    unless ($Config{ivsize} and $Config{ivsize} > $Config{longsize}) {
        print "1..0 # Skip: Your IVs are no larger than your longs\n";
        exit 0;
    }
}

use Storable qw(freeze thaw);
use strict;
use Test::More tests=>30;

our (%tests);

{
    local $/ = "\n\nend\n";
    while (<DATA>) {
        next unless /\S/s;
        unless (/begin ([0-7]{3}) ([^\n]*)\n(.*)$/s) {
            s/\n.*//s;
            warn "Dodgy data in section starting '$_'";
            next;
        }
        next unless oct $1 == ord 'A'; # Skip ASCII on EBCDIC, and vice versa
        my $data = unpack 'u', $3;
        $tests{$2} = $data;
    }
}

# perl makes easy things easy, and hard things possible:
my $test = freeze \'Hell';

my $header = Storable::read_magic ($test);

is ($header->{byteorder}, $Config{byteorder},
    "header's byteorder and Config.pm's should agree");

my $result = eval {thaw $test};
isa_ok ($result, 'SCALAR', "Check thawing test data");
is ($@, '', "causes no errors");
is ($$result, 'Hell', 'and gives the expected data');

my $kingdom = $Config{byteorder} =~ /23/ ? "Lillput" : "Belfuscu";

my $name = join ',', $kingdom, @$header{qw(intsize longsize ptrsize nvsize)};

SKIP: {
    my $real_thing = $tests{$name};
    if (!defined $real_thing) {
        print << "EOM";
# No test data for Storable 1.x for:
#
# byteorder	 '$Config{byteorder}'
# sizeof(int)	 $$header{intsize}
# sizeof(long)	 $$header{longsize}
# sizeof(char *) $$header{ptrsize}
# sizeof(NV)	 $$header{nvsize}

# If you have Storable 1.x built with perl 5.6.x on this platform, please
# make_56_interwork.pl to generate test data, and append the test data to
# this test. 
# You may find that make_56_interwork.pl reports that your platform has no
# interworking problems, in which case you need do nothing.
EOM
        skip "# No 1.x test file", 9;
    }
    my $result = eval {thaw $real_thing};
    is ($result, undef, "By default should not be able to thaw");
    like ($@, qr/Byte order is not compatible/,
          "because the header byte order strings differ");
    local $Storable::interwork_56_64bit = 1;
    $result = eval {thaw $real_thing};
    isa_ok ($result, 'ARRAY', "With flag should now thaw");
    is ($@, '', "with no errors");

    # However, as the file is written with Storable pre 2.01, it's a known
    # bug that large (positive) UVs become IVs
    my $value = (~0 ^ (~0 >> 1) ^ 2);

    is (@$result, 4, "4 elements in array");
    like ($$result[0],
          qr/^This file was written with [0-9.]+ on perl [0-9.]+\z/,
         "1st element");
    is ($$result[1], "$kingdom was correct", "2nd element");
    cmp_ok ($$result[2] ^ $value, '==', 0, "3rd element") or
        printf "# expected %#X, got %#X\n", $value, $$result[2];
    is ($$result[3], "The End", "4th element");
}

$result = eval {thaw $test};
isa_ok ($result, 'SCALAR', "CHORUS: check thawing test data");
is ($@, '', "        causes no errors");
is ($$result, 'Hell', "        and gives the expected data");

my $test_kludge;
{
    local $Storable::interwork_56_64bit = 1;
    $test_kludge = freeze \'Heck';
}

my $header_kludge = Storable::read_magic ($test_kludge);

cmp_ok (length ($header_kludge->{byteorder}), '==', $Config{longsize},
        "With 5.6 interwork kludge byteorder string should be same size as long"
       );
$result = eval {thaw $test_kludge};
is ($result, undef, "By default should not be able to thaw");
like ($@, qr/Byte order is not compatible/,
      "because the header byte order strings differ");

$result = eval {thaw $test};
isa_ok ($result, 'SCALAR', "CHORUS: check thawing test data");
is ($@, '', "        causes no errors");
is ($$result, 'Hell', "        and gives the expected data");

{
    local $Storable::interwork_56_64bit = 1;

    $result = eval {thaw $test_kludge};
    isa_ok ($result, 'SCALAR', "should be able to thaw kludge data");
    is ($@, '', "with no errors");
    is ($$result, 'Heck', "and gives expected data");

    $result = eval {thaw $test};
    is ($result, undef, "But now can't thaw real data");
    like ($@, qr/Byte order is not compatible/,
          "because the header byte order strings differ");
}

#  All together now:
$result = eval {thaw $test};
isa_ok ($result, 'SCALAR', "CHORUS: check thawing test data");
is ($@, '', "        causes no errors");
is ($$result, 'Hell', "        and gives the expected data");

__END__
# A whole run of 1.1.14 freeze data, uuencoded. The "mode bits" are the octal
# value of 'A', the "file name" is the test name. Use make_56_interwork.pl
# with a copy of Storable 1.X generate these.

# byteorder      '1234'
# sizeof(int)    4
# sizeof(long)   4
# sizeof(char *) 4
# sizeof(NV)     8
begin 101 Lillput,4,4,4,8
M!`0$,3(S-`0$!`@"!`````HQ5&AI<R!F:6QE('=A<R!W<FET=&5N('=I=&@@
M,2XP,30@;VX@<&5R;"`U+C`P-C`P,0H33&EL;'!U="!W87,@8V]R<F5C=`8"
0````````@`H'5&AE($5N9```

end

# byteorder      '4321'
# sizeof(int)    4
# sizeof(long)   4
# sizeof(char *) 4
# sizeof(NV)     8
begin 101 Belfuscu,4,4,4,8
M!`0$-#,R,00$!`@"````!`HQ5&AI<R!F:6QE('=A<R!W<FET=&5N('=I=&@@
M,2XP,30@;VX@<&5R;"`U+C`P-C`P,0H40F5L9G5S8W4@=V%S(&-O<G)E8W0&
1@`````````(*!U1H92!%;F0`

end

# byteorder      '1234'
# sizeof(int)    4
# sizeof(long)   4
# sizeof(char *) 4
# sizeof(NV)     12
begin 101 Lillput,4,4,4,12
M!`0$,3(S-`0$!`P"!`````HQ5&AI<R!F:6QE('=A<R!W<FET=&5N('=I=&@@
M,2XP,30@;VX@<&5R;"`U+C`P-C`P,0H33&EL;'!U="!W87,@8V]R<F5C=`8"
0````````@`H'5&AE($5N9```

end

