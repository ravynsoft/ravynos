#!./perl

print "1..7\n";
my $test = 0;

sub is {
    my ($got, $expect, $name) = @_;
    $test = $test + 1;
    if (defined $got && $got eq $expect) {
	print "ok $test - $name\n";
	return 1;
    }

    print "not ok $test - $name\n";
    my @caller = caller(0);
    print "# Failed test at $caller[1] line $caller[2]\n";
    if (defined $got) {
	print "# Got '$got'\n";
    } else {
	print "# Got undef\n";
    }
    print "# Expected $expect\n";
    return;
}

{
    package TieAll;
    # tie, track, and report what calls are made
    my @calls;
    sub AUTOLOAD {
        for ($AUTOLOAD =~ /TieAll::(.*)/) {
            if (/TIE/) { return bless {} }
            elsif (/calls/) { return join ',', splice @calls }
            else {
               push @calls, $_;
	       # FETCHSIZE doesn't like undef
	       # if FIRSTKEY, see if NEXTKEY is also called
               return 1 if /FETCHSIZE|FIRSTKEY/;
               return;
            }
        }
    }
}

tie $x, 'TieAll';
tie @x, 'TieAll';
tie %x, 'TieAll';

{our $x;}
is(TieAll->calls, '', 'our $x has no runtime effect');

{our ($x);}
is(TieAll->calls, '', 'our ($x) has no runtime effect');

{our %x;}
is(TieAll->calls, '', 'our %x has no runtime effect');

{our (%x);}
is(TieAll->calls, '', 'our (%x) has no runtime effect');

{our @x;}
is(TieAll->calls, '', 'our @x has no runtime effect');

{our (@x);}
is(TieAll->calls, '', 'our (@x) has no runtime effect');


$y = 1;
{
    my $y = 2;
    {
	our $y = $y;
	is($y, 2, 'our shouldnt be visible until introduced')
    }
}
