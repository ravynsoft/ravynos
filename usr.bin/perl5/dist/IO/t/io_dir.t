#!./perl

use strict;
use File::Temp qw( tempdir );
use Cwd;

no strict 'subs';

BEGIN {
    require($ENV{PERL_CORE} ? "../../t/test.pl" : "./t/test.pl");
    plan(16);

    use_ok('IO::Dir');
    IO::Dir->import(DIR_UNLINK);
}

my $cwd = cwd();

{
    my $DIR = tempdir( CLEANUP => 1 );
    chdir $DIR or die "Unable to chdir to $DIR";
    my @IO_files =
        ( 'ChangeLog', 'IO.pm', 'IO.xs', 'Makefile.PL', 'poll.c', 'poll.h', 'README' );
    my @IO_subdirs = ( qw| hints  lib  t | );

    for my $f (@IO_files) {
        open my $OUT, '>', $f or die "Unable to open '$DIR/$f' for writing";
        close $OUT or die "Unable to close '$DIR/$f' after writing";
    }
    for my $d (@IO_subdirs) { mkdir $d or die "Unable to mkdir '$DIR/$d'"; }

    my $CLASS = "IO::Dir";
    my $dot = $CLASS->new($DIR);
    ok(defined($dot), "Able to create IO::Dir object for $DIR");

    my @a = sort <*>;
    my $first;
    do { $first = $dot->read } while defined($first) && $first =~ /^\./;
    ok(+(grep { $_ eq $first } @a), "directory entry found");

    my @b = sort($first, (grep {/^[^.]/} $dot->read));
    ok(+(join("\0", @a) eq join("\0", @b)), "two lists of directory entries match (Case 1)");

    ok($dot->rewind,'rewind');
    my @c = sort grep {/^[^.]/} $dot->read;
    ok(+(join("\0", @b) eq join("\0", @c)), "two lists of directory entries match (Case 2)");

    ok($dot->close,'close');
    {
        local $^W; # avoid warnings on invalid dirhandle
        ok(!$dot->rewind, "rewind on closed");
        ok(!defined($dot->read), "Directory handle closed; 'read' returns undef");
    }

    open(FH,'>','X') || die "Can't create x";
    print FH "X";
    close(FH) or die "Can't close: $!";

    my %dir;
    tie %dir, $CLASS, $DIR;
    my @files = keys %dir;

    # I hope we do not have an empty dir :-)
    ok(scalar @files, "Tied hash interface finds directory entries");

    my $stat = $dir{'X'};
    isa_ok($stat,'File::stat');
    ok(defined($stat) && $stat->size == 1,
        "Confirm that we wrote a file of size 1 byte");

    delete $dir{'X'};

    ok(-f 'X', "File still exists after tied hash entry deleted");

    my %dirx;
    tie %dirx, $CLASS, $DIR, DIR_UNLINK;

    my $statx = $dirx{'X'};
    isa_ok($statx,'File::stat');
    ok(defined($statx) && $statx->size == 1,
        "Confirm that we still have the 1-byte file");

    delete $dirx{'X'};

    ok(!(-f 'X'), "Using DIR_UNLINK deletes tied hash element and directory entry");

    chdir $cwd or die "Unable to chdir back to $cwd";
}

