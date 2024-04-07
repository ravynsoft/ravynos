#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib', '..');
}

use TestInit qw(T);    # T is chdir to the top level
use strict;

find_git_or_skip('all');

my $ok= do "./Porting/updateAUTHORS.pl";
my $error= !$ok && $@;
is($ok,1,"updateAUTHORS.pl compiles correctly");
is($error, "", "updateAUTHORS.pl compiles without error");
my $small_range= "544171f79ec3e50bb5003007e9f4ebb9a7e9fe84^^^"
               . "..544171f79ec3e50bb5003007e9f4ebb9a7e9fe84";
my $large_range= "6d02a9e121d037896df9b91ac623c1ab4c98c99a.."
               . "544171f79ec3e50bb5003007e9f4ebb9a7e9fe8";
my $with_unknown_range= "96a91e01636d3050d38ae3373a362c7d47a6647e^^^.."
                      . "96a91e01636d3050d38ae3373a362c7d47a6647e";

foreach my $tuple (
    [ "--who", $small_range,
               "James E Keenan, Karl Williamson, Mark Shelor." ],
    [ "--files", $small_range, files_expected() ],
    [ "--rank", $large_range, rank_expected()],
    [ "--rank --percentage", $large_range, rank_percentage_expected()],
    [ "--rank --percentage --cumulative", $large_range,
        rank_percentage_cumulative_expected()],
    [ "--thanks-applied", $large_range, thanks_applied_expected() ],
    [ "--stats", $large_range, stats_expected() ],
    [ "--stats --numstat", $large_range, stats_numstat_expected() ],
    [ "--who" , $with_unknown_range, "Jarkko Hietaniemi.", "(no 'unknown' authors)" ],
) {
    my ($arg,$range,$expect, $msg_extra)= @$tuple;
    my $skip_it;
    for my $endpoint (split /\.+/, $range) {
        my $parsed= `git rev-parse --verify -q $endpoint\{commit}`;
        if (!$parsed or $?) {
            $skip_it = 1;
        }
    }
    SKIP: {
        if ($skip_it) {
            skip "commit range '$range' not available (this happens in CI)", 1;
        }
        $msg_extra= $msg_extra ? " $msg_extra" : "";
        my $cmd= join " ", "$^X ./Porting/updateAUTHORS.pl",
                           $arg, $range;
        my $result= `$cmd`;
        is(_clean($result), _clean($expect),"Option '$arg' works as expected$msg_extra")
            or print STDERR "$cmd\n",$result
    }
}
done_testing();
exit 0;
sub _clean {
    my ($str)= @_;
    $str=~s/\s+\z//;
    $str=~s/[ ]+\n/\n/g;
    return $str;
}

sub files_expected {
    return <<'END_OF_REPORT';
#Pos | commits | L++ | L-- | L+- | binary_change | Name
#----+---------+-----+-----+-----+---------------+----------------------------------
#1   |       1 |  28 |   0 |  28 |             0 | pod/perlfunc.pod
#2   |       1 |  14 |   4 |  10 |             0 | cpan/Digest-SHA/lib/Digest/SHA.pm
#3   |       1 |   5 |   5 |   0 |             0 | cpan/Digest-SHA/shasum
#4   |       1 |   3 |   3 |   0 |             0 | cpan/Digest-SHA/src/sha64bit.c
#5   |       1 |   3 |   3 |   0 |             0 | cpan/Digest-SHA/src/sha64bit.h
#6   |       1 |   3 |   3 |   0 |             0 | cpan/Digest-SHA/src/sha.c
#7   |       1 |   3 |   3 |   0 |             0 | cpan/Digest-SHA/src/sha.h
#8   |       1 |   1 |   1 |   0 |             0 | Porting/Maintainers.pl
#9   |       1 |   1 |   0 |   1 |             0 | AUTHORS
END_OF_REPORT
}

sub rank_expected {
    return <<'END_OF_REPORT';
#Pos | Authored | Name
#----+----------+-----------------
#1   |       40 | Karl Williamson
#2   |       32 | Yves Orton
#3   |        8 | Paul Evans
#4   |        6 | James E Keenan
#5   |        4 | Elvin Aslanov
#6   |        3 | Richard Leach
#7   |        3 | Tony Cook
#8   |        2 | Nicholas Clark
#9   |        1 | Dan Kogai
#10  |        1 | David Golden
#11  |        1 | Graham Knop
#12  |        1 | Mark Shelor
#13  |        1 | Tomasz Konojacki
END_OF_REPORT
}

sub rank_percentage_expected {
    return <<'END_OF_REPORT';
#Pos | %Authored | Name
#----+-----------+-----------------
#1   |     38.83 | Karl Williamson
#2   |     31.07 | Yves Orton
#3   |      7.77 | Paul Evans
#4   |      5.83 | James E Keenan
#5   |      3.88 | Elvin Aslanov
#6   |      2.91 | Richard Leach
#7   |      2.91 | Tony Cook
#8   |      1.94 | Nicholas Clark
#9   |      0.97 | Dan Kogai
#10  |      0.97 | David Golden
#11  |      0.97 | Graham Knop
#12  |      0.97 | Mark Shelor
#13  |      0.97 | Tomasz Konojacki
END_OF_REPORT
}

sub rank_percentage_cumulative_expected {
    return <<'END_OF_REPORT';
#Pos | %Authored | Name
#----+-----------+-----------------
#1   |     38.83 | Karl Williamson
#2   |     69.90 | Yves Orton
#3   |     77.67 | Paul Evans
#4   |     83.50 | James E Keenan
#5   |     87.38 | Elvin Aslanov
#6   |     90.29 | Richard Leach
#7   |     93.20 | Tony Cook
#8   |     95.15 | Nicholas Clark
#9   |     96.12 | Dan Kogai
#10  |     97.09 | David Golden
#11  |     98.06 | Graham Knop
#12  |     99.03 | Mark Shelor
#13  |    100.00 | Tomasz Konojacki
END_OF_REPORT
}

sub thanks_applied_expected {
    return <<'END_OF_REPORT';
#Pos | Applied | Name
#----+---------+----------------
#1   |       7 | Karl Williamson
#2   |       4 | James E Keenan
END_OF_REPORT
}

sub stats_expected {
    return <<'END_OF_REPORT';
#Pos | Authored | Applied | Committed | Name
#----+----------+---------+-----------+-----------------
#1   |       40 |       7 |        47 | Karl Williamson
#2   |       32 |       0 |        31 | Yves Orton
#3   |        8 |       0 |         8 | Paul Evans
#4   |        6 |       4 |        10 | James E Keenan
#5   |        4 |       0 |         1 | Elvin Aslanov
#6   |        3 |       0 |         3 | Tony Cook
#7   |        3 |       0 |         0 | Richard Leach
#8   |        2 |       0 |         2 | Nicholas Clark
#9   |        1 |       0 |         1 | Tomasz Konojacki
#10  |        1 |       0 |         0 | Dan Kogai
#11  |        1 |       0 |         0 | David Golden
#12  |        1 |       0 |         0 | Graham Knop
#13  |        1 |       0 |         0 | Mark Shelor
END_OF_REPORT
}

sub stats_numstat_expected {
    return <<'END_OF_REPORT';
#Pos | Authored | Applied | Committed | NFiles |  L++ |  L-- |  L+- | Name
#----+----------+---------+-----------+--------+------+------+------+-----------------
#1   |       40 |       7 |        47 |     14 | 1179 |  874 |  305 | Karl Williamson
#2   |       32 |       0 |        31 |     25 | 2547 | 1481 | 1066 | Yves Orton
#3   |        8 |       0 |         8 |     15 |  161 |  102 |   59 | Paul Evans
#4   |        6 |       4 |        10 |      4 |   44 |   11 |   33 | James E Keenan
#5   |        4 |       0 |         1 |      4 |   16 |   13 |    3 | Elvin Aslanov
#6   |        3 |       0 |         3 |      7 |    8 |    7 |    1 | Tony Cook
#7   |        3 |       0 |         0 |     13 |   75 |   51 |   24 | Richard Leach
#8   |        2 |       0 |         2 |      2 |   24 |    1 |   23 | Nicholas Clark
#9   |        1 |       0 |         1 |      2 |   21 |   15 |    6 | Tomasz Konojacki
#10  |        1 |       0 |         0 |      8 |   33 |   22 |   11 | Mark Shelor
#11  |        1 |       0 |         0 |      5 |   93 |    7 |   86 | Graham Knop
#12  |        1 |       0 |         0 |      4 |    9 |    4 |    5 | Dan Kogai
#13  |        1 |       0 |         0 |      2 |   19 |    6 |   13 | David Golden
END_OF_REPORT
}
