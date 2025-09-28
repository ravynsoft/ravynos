#!./perl -w

no strict;

BEGIN {
    if ($ENV{PERL_CORE}) {
	@INC = '../lib';
	chdir 't';
    }
}

use Getopt::Long qw(GetOptionsFromString :config no_ignore_case);
my $want_version="2.3501";
die("Getopt::Long version $want_version required--this is only version ".
    $Getopt::Long::VERSION)
  unless $Getopt::Long::VERSION ge $want_version;

print "1..14\n";

my $args = "-Foo -baR --foo";
@ARGV = qw(foo bar);
undef $opt_baR;
undef $opt_bar;
print (GetOptionsFromString($args, "foo", "Foo=s") ? "" : "not ", "ok 1\n");
print ((defined $opt_foo)   ? "" : "not ", "ok 2\n");
print (($opt_foo == 1)      ? "" : "not ", "ok 3\n");
print ((defined $opt_Foo)   ? "" : "not ", "ok 4\n");
print (($opt_Foo eq "-baR") ? "" : "not ", "ok 5\n");
print (!(defined $opt_baR)  ? "" : "not ", "ok 6\n");
print (!(defined $opt_bar)  ? "" : "not ", "ok 7\n");
print ("@ARGV" eq "foo bar" ? "" : "not ", "ok 8\n");

$args = "-Foo -baR blech --foo bar";
@ARGV = qw(foo bar);
undef $opt_baR;
undef $opt_bar;
{ my $msg = "";
  local $SIG{__WARN__} = sub { $msg .= "@_" };
  my $ret = GetOptionsFromString($args, "foo", "Foo=s");
  print ($ret ? "not " : "ok 9\n");
  print ($msg =~ /^GetOptionsFromString: Excess data / ? "" : "$msg\nnot ", "ok 10\n");
}
print ("@ARGV" eq "foo bar" ? "" : "not ", "ok 11\n");

$args = "-Foo -baR blech --foo bar";
@ARGV = qw(foo bar);
undef $opt_baR;
undef $opt_bar;
{ my $ret;
  ($ret, $args) = GetOptionsFromString($args, "foo", "Foo=s");
  print ($ret ? "" : "not ", "ok 12\n");
  print ("@$args" eq "blech bar" ? "" : "@$args\nnot ", "ok 13\n");
}
print ("@ARGV" eq "foo bar" ? "" : "not ", "ok 14\n");
