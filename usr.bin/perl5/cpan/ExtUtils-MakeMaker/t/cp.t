#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib/';
}
chdir 't';

use strict;
use warnings;
use ExtUtils::Command;
use Test::More tests => 1;

open FILE, ">source" or die $!;
print FILE "stuff\n";
close FILE;

# Instead of sleeping to make the file time older
utime time - 900, time - 900, "source";

END { 1 while unlink "source", "dest"; }

# Win32 bug, cp wouldn't update mtime.
{
    local @ARGV = qw(source dest);
    cp();
    my $mtime = (stat("dest"))[9];
    my $now   = time;
    cmp_ok( abs($mtime - $now), '<=', 1, 'cp updated mtime' );
}
