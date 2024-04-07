#!./perl

use strict;
use warnings;

use Test::More tests => 21;
use Scalar::Util qw(openhandle);

ok(defined &openhandle, 'defined');

{
    my $fh = \*STDERR;
    is(openhandle($fh), $fh, 'STDERR');

    is(fileno(openhandle(*STDERR)), fileno(STDERR), 'fileno(STDERR)');
}

{
    use vars qw(*CLOSED);
    is(openhandle(*CLOSED), undef, 'closed');
}

SKIP: {
    skip "3-arg open only on 5.6 or later", 1 if $]<5.006;

    open my $fh, "<", $0;
    skip "could not open $0 for reading: $!", 2 unless $fh;
    is(openhandle($fh), $fh, "works with indirect filehandles");
    close($fh);
    is(openhandle($fh), undef, "works with indirect filehandles");
}

SKIP: {
    skip "in-memory files only on 5.8 or later", 2 if $]<5.008;

    open my $fh, "<", \"in-memory file";
    skip "could not open in-memory file: $!", 2 unless $fh;
    is(openhandle($fh), $fh, "works with in-memory files");
    close($fh);
    is(openhandle($fh), undef, "works with in-memory files");
}

ok(openhandle(\*DATA), "works for \*DATA");
ok(openhandle(*DATA), "works for *DATA");
ok(openhandle(*DATA{IO}), "works for *DATA{IO}");

{
    require IO::Handle;
    my $fh = IO::Handle->new_from_fd(fileno(*STDERR), 'w');
    skip "new_from_fd(fileno(*STDERR)) failed", 2 unless $fh;
    ok(openhandle($fh), "works for IO::Handle objects");

    ok(!openhandle(IO::Handle->new), "unopened IO::Handle");
}

{
    require IO::File;
    my $fh = IO::File->new;
    $fh->open("< $0")
        or skip "could not open $0: $!", 3;
    ok(openhandle($fh), "works for IO::File objects");
    close($fh);
    ok(!openhandle($fh), "works for IO::File objects");

    ok(!openhandle(IO::File->new), "unopened IO::File" );
}

SKIP: {
    skip( "Tied handles only on 5.8 or later", 2) if $]<5.008;

    use vars qw(*H);

    package My::Tie;
    require Tie::Handle;
    @My::Tie::ISA = qw(Tie::Handle);
    sub TIEHANDLE { bless {} }

    package main;
    tie *H, 'My::Tie';
    ok(openhandle(*H), "tied handles are always ok");
    ok(openhandle(\*H), "tied handle refs are always ok");
}

ok !openhandle(undef),   "undef is not a filehandle";
ok !openhandle("STDIN"), "strings are not filehandles";
ok !openhandle(0),       "integers are not filehandles";


__DATA__
