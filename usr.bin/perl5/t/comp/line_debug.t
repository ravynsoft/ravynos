#!./perl

BEGIN { unshift @INC, '.' }

chdir 't' if -d 't';

sub ok {
    my($test,$ok) = @_;
    print "not " unless $ok;
    print "ok $test\n";
    $ok;
}

# The auxiliary file contains a bunch of code that systematically exercises
# every place that can call lex_next_chunk() (except for the one that's not
# used by the main Perl parser).
open AUX, "<", "comp/line_debug_0.aux" or die $!;
my @lines = <AUX>;
close AUX;
my $nlines = @lines;

print "1..", 2+$nlines, "\n";

$^P = 0x2;
do "comp/line_debug_0.aux";

ok 1, scalar(@{"_<comp/line_debug_0.aux"}) == 1+$nlines;
ok 2, !defined(${"_<comp/line_debug_0.aux"}[0]);

for(1..$nlines) {
    if (!ok 2+$_, ${"_<comp/line_debug_0.aux"}[$_] eq $lines[$_-1]) {
	print "# Got: ", ${"_<comp/line_debug_0.aux"}[$_]//"undef\n";
	print "# Expected: $lines[$_-1]";
    }
}

1;
