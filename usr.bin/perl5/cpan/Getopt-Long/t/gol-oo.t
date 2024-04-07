#!./perl -w

no strict;

BEGIN {
    if ($ENV{PERL_CORE}) {
	@INC = '../lib';
	chdir 't';
    }
}

use Getopt::Long;
my $want_version="2.24";
die("Getopt::Long version $want_version required--this is only version ".
    $Getopt::Long::VERSION)
  unless $Getopt::Long::VERSION ge $want_version;
print "1..14\n";

@ARGV = qw(-Foo -baR --foo bar);
my $p = new Getopt::Long::Parser (config => ["no_ignore_case"]);
undef $opt_baR;
undef $opt_bar;
print "ok 1\n" if $p->getoptions ("foo", "Foo=s");
print ((defined $opt_foo)   ? "" : "not ", "ok 2\n");
print (($opt_foo == 1)      ? "" : "not ", "ok 3\n");
print ((defined $opt_Foo)   ? "" : "not ", "ok 4\n");
print (($opt_Foo eq "-baR") ? "" : "not ", "ok 5\n");
print ((@ARGV == 1)         ? "" : "not ", "ok 6\n");
print (($ARGV[0] eq "bar")  ? "" : "not ", "ok 7\n");
print (!(defined $opt_baR)  ? "" : "not ", "ok 8\n");
print (!(defined $opt_bar)  ? "" : "not ", "ok 9\n");

my @args = (qw[-test 1]);
my $o = Getopt::Long::Parser->new;
print "ok 10\n" if $o->getoptionsfromarray(\@args, "test=i");
print ((defined $opt_test) ? "" : "not ", "ok 11\n");
print (($opt_test == 1)    ? "" : "not ", "ok 12\n");
print ((@ARGV == 1)        ? "" : "not ", "ok 13\n");
print ((@args == 0)        ? "" : "not ", "ok 14\n");
