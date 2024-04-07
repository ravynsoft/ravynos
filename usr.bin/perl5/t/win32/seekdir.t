#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require "./test.pl";
}

use warnings;
use strict;
use Errno;

{ # Test we can seekdir to all positions.

    my $dh;
    ok(opendir($dh, ".") == 1, "able to opendir('.')");

    # Build up a list of all the files and their positions.
    my @p_f;  # ([POS_0, FILE_0], [POS_1, FILE_1], ...)
    while (1) {
        my $p = telldir $dh;
        my $f = readdir $dh;
        last unless defined $f;
        push @p_f, [$p, $f];
    }

    # Test we can seekdir() to the given position and that
    # readdir() returns the expected file name.
    my $test = sub {
        my ($p_f, $type) = @_;
        my ($p, $f) = @$p_f;
        ok(seekdir($dh, $p), "$type seekdir($p)");
        ok(readdir($dh) eq $f, "$type readdir() -> $f \tas expected");
    };
    # Go forwards.
    $test->($_, "forward") for @p_f;
    # Go backwards.
    $test->($_, "backward") for reverse @p_f;
    # A mixed traversal: longest file names first.
    my @sorted_p_f = sort {
            length $b->[1] <=> length $a->[1]
                or
            $a->[1] cmp $b->[1]
    } @p_f;
    $test->($_, "mixed") for @sorted_p_f;

    # Test behaviour of seekdir(-1).
    ok(seekdir($dh, -1), "seekdir(-1) returns true...");
    ok(!defined readdir($dh), "...but next readdir() gives undef");

    # Test behaviour of seekdir() to a position beyond what we
    # have read so far.
    my $final_p_f = $p_f[-1];
    my $end_pos = $final_p_f->[0] + length $final_p_f->[1];
    ok(seekdir($dh, $end_pos), "seekdir($end_pos) possible");
    ok(telldir($dh) == $end_pos, "telldir() equal to where we seekdir()d");
    # At this point we readdir() the trailing NUL of the last file name.
    ok(readdir($dh) eq '', "readdir() here gives an empty string");

    # Reached the end of files to seekdir() to.
    ok(telldir($dh) == -1, "telldir() now equal to -1");
    ok(!defined readdir($dh), "next readdir() gives undef");

    # NB. `seekdir(DH, POS)` always returns true regardless of the
    # value of POS, providing DH is a valid directory handle.
    # However, if POS _is_ out of range then `telldir(DH)` is -1,
    # and `readdir(DH)` returns undef.
    ok(seekdir($dh, $end_pos + 1), "seekdir($end_pos + 1) returns true...");
    ok(telldir($dh) == -1, "....but telldir() == -1 indicating out of range");
    ok(!defined readdir($dh), "... and next readdir() gives undef");

    ok(closedir($dh) == 1, "Finally. closedir() returns true");
}

done_testing();
