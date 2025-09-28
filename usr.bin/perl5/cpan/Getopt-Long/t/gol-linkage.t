#!./perl -w

no strict;

BEGIN {
    if ($ENV{PERL_CORE}) {
	@INC = '../lib';
	chdir 't';
    }
}

use Getopt::Long;

print "1..37\n";

@ARGV = qw(-Foo -baR --foo bar);
Getopt::Long::Configure ("no_ignore_case");
%lnk = ();
print "ok 1\n" if GetOptions (\%lnk, "foo", "Foo=s");
print ((defined $lnk{foo})   ? "" : "not ", "ok 2\n");
print (($lnk{foo} == 1)      ? "" : "not ", "ok 3\n");
print ((defined $lnk{Foo})   ? "" : "not ", "ok 4\n");
print (($lnk{Foo} eq "-baR") ? "" : "not ", "ok 5\n");
print ((@ARGV == 1)          ? "" : "not ", "ok 6\n");
print (($ARGV[0] eq "bar")   ? "" : "not ", "ok 7\n");
print (!(exists $lnk{baR})   ? "" : "not ", "ok 8\n");

@ARGV = qw(-Foo -baR --foo bar);
Getopt::Long::Configure ("default","no_ignore_case");
%lnk = ();
my $foo;
print "ok 9\n" if GetOptions (\%lnk, "foo" => \$foo, "Foo=s");
print ((defined $foo)        ? "" : "not ", "ok 10\n");
print (($foo == 1)           ? "" : "not ", "ok 11\n");
print ((defined $lnk{Foo})   ? "" : "not ", "ok 12\n");
print (($lnk{Foo} eq "-baR") ? "" : "not ", "ok 13\n");
print ((@ARGV == 1)          ? "" : "not ", "ok 14\n");
print (($ARGV[0] eq "bar")   ? "" : "not ", "ok 15\n");
print (!(exists $lnk{foo})   ? "" : "not ", "ok 16\n");
print (!(exists $lnk{baR})   ? "" : "not ", "ok 17\n");
print (!(exists $lnk{bar})   ? "" : "not ", "ok 18\n");

@ARGV = qw(/Foo=-baR --bar bar);
Getopt::Long::Configure ("default","prefix_pattern=--|/|-|\\+","long_prefix_pattern=--|/");
%lnk = ();
my $bar;
print "ok 19\n" if GetOptions (\%lnk, "bar" => \$bar, "Foo=s");
print ((defined $bar)        ? "" : "not ", "ok 20\n");
print (($bar == 1)           ? "" : "not ", "ok 21\n");
print ((defined $lnk{Foo})   ? "" : "not ", "ok 22\n");
print (($lnk{Foo} eq "-baR") ? "" : "not ", "ok 23\n");
print ((@ARGV == 1)          ? "" : "not ", "ok 24\n");
print (($ARGV[0] eq "bar")   ? "" : "not ", "ok 25\n");
print (!(exists $lnk{foo})   ? "" : "not ", "ok 26\n");
print (!(exists $lnk{baR})   ? "" : "not ", "ok 27\n");
print (!(exists $lnk{bar})   ? "" : "not ", "ok 28\n");
{
    my $errors;
    %lnk = ();
    local $SIG{__WARN__}= sub { $errors.=join("\n",@_,'') };

    @ARGV = qw(/Foo=-baR);
    Getopt::Long::Configure ("default","bundling","ignore_case_always",
                             "prefix_pattern=--|/|-|\\+","long_prefix_pattern=--");
    %lnk = ();
    undef $bar;
    GetOptions (\%lnk, "bar" => \$bar, "Foo=s");
    print (($errors=~/Unknown option:/) ? "" : "not ", "ok 29\n");
    $errors="";
    %lnk = ();
    undef $bar;
     @ARGV = qw(/Foo=-baR);
    Getopt::Long::Configure ("default","bundling","ignore_case_always",
                             "prefix_pattern=--|/|-|\\+","long_prefix_pattern=--|/");
    GetOptions (\%lnk, "bar" => \$bar, "Foo=s");
    print (($errors eq '') ? "" : "not ", "ok 30\n");
    print ((defined $lnk{Foo})   ? "" : "not ", "ok 31\n");
    print (($lnk{Foo} eq "-baR") ? "" : "not ", "ok 32\n");
}

{
    # Allow hashes to overload "".
    # This used to fail up to 2.34.
    # Thanks to Yves Orton.
    my $blessed = bless(\%lnk, "OverLoad::Test");

    @ARGV = qw(--foo bar);
    Getopt::Long::Configure("default");
    print "not" unless GetOptions (\%lnk, "foo=s" => \$foo);
    print "ok 33\n";
    package Overload::Test;
    use overload '""' => sub{ die "Bad mojo!" };
}

{
    @ARGV = (qw[-thru -here -more 1]);
    my $got = "";
    Getopt::Long::Configure("default");
    Getopt::Long::Configure("pass_through");
    print "not" unless GetOptions
	("here" => sub { $got .= " sub_here"; },
	 "<>" => sub { $got .= " <>=".$_[0]; }, );
    $got .= " remain=".join(',',@ARGV);
    print "ok 34\n";
    print +(($got eq " <>=-thru sub_here <>=-more <>=1 remain=")
	    ? "" : "not ", "ok 35\n");
}

{
    @ARGV = (qw[-thru -here -more -- 1]);
    my $got = "";
    Getopt::Long::Configure("default");
    Getopt::Long::Configure("pass_through","require_order");
    print "not" unless GetOptions
	("here" => sub { $got .= " sub_here"; },
	 "<>" => sub { $got .= " <>=".$_[0]; }, );
    $got .= " remain=".join(',',@ARGV);
    print "ok 36\n";
    print +(($got eq " remain=-thru,-here,-more,--,1")
	    ? "" : "not ", "ok 37\n");
}
