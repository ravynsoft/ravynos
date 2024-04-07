#!./perl -w

no strict;

BEGIN {
    if ($ENV{PERL_CORE}) {
	@INC = '../lib';
	chdir 't';
    }
}

use Getopt::Long qw(:config no_ignore_case);
my $want_version="2.24";
die("Getopt::Long version $want_version required--this is only version ".
    $Getopt::Long::VERSION)
  unless $Getopt::Long::VERSION ge $want_version;

print "1..18\n";

@ARGV = qw(-Foo -baR --foo bar);
undef $opt_baR;
undef $opt_bar;
print (GetOptions("foo", "Foo=s") ? "" : "not ", "ok 1\n");
print ((defined $opt_foo)   ? "" : "not ", "ok 2\n");
print (($opt_foo == 1)      ? "" : "not ", "ok 3\n");
print ((defined $opt_Foo)   ? "" : "not ", "ok 4\n");
print (($opt_Foo eq "-baR") ? "" : "not ", "ok 5\n");
print ((@ARGV == 1)         ? "" : "not ", "ok 6\n");
print (($ARGV[0] eq "bar")  ? "" : "not ", "ok 7\n");
print (!(defined $opt_baR)  ? "" : "not ", "ok 8\n");
print (!(defined $opt_bar)  ? "" : "not ", "ok 9\n");

# Tests added, see https://rt.cpan.org/Ticket/Display.html?id=87581
@ARGV = qw(--help --file foo --foo --nobar --num=5 -- file);
$rv  = GetOptions(
	'help'   => \$HELP,
	'file:s' => \$FILE,
	'foo!'   => \$FOO,
	'bar!'   => \$BAR,
	'num:i'  => \$NO,
    );
print ($rv ? "" : "not "); print "ok 10\n";
print ("@ARGV" eq 'file' ? "" : "not ", "ok 11\n");
( $HELP && $FOO && !$BAR && $FILE eq 'foo' && $NO == 5 )
    ? print "" : print "not "; print "ok 12\n";

# Test behaviour when the same option name is given twice, but not an multi-value option.
# The option given later on the command line is used.
#
{
    my $foo;

    @ARGV = qw(--foo a --foo b);
    $rd = GetOptions('foo=s' => \$foo);
    print ($rv ? "" : "not "); print "ok 13\n";
    print ($foo eq 'b' ? "" : "not ", "ok 14\n");

    @ARGV = qw(--no-foo --foo);
    $rd = GetOptions('foo!' => \$foo);
    print ($rv ? "" : "not "); print "ok 15\n";
    print ($foo eq '1' ? "" : "not ", "ok 16\n");

    @ARGV = qw(--foo --no-foo);
    $rd = GetOptions('foo!' => \$foo);
    print ($rv ? "" : "not "); print "ok 17\n";
    # Check it is set to an explicit 0.
    print ($foo eq '0' ? "" : "not ", "ok 18\n");
}
