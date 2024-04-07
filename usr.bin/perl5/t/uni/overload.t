#!perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( '../lib' );
    require Config; import Config;
    require './charset_tools.pl';
    require './loc_tools.pl';
}

plan(tests => 193);

package UTF8Toggle;
use strict;

use overload '""' => 'stringify', fallback => 1;

sub new {
    my $class = shift;
    my $value = shift;
    my $state = shift||0;
    return bless [$value, $state], $class;
}

sub stringify {
    my $self = shift;
    $self->[1] = ! $self->[1];
    if ($self->[1]) {
	utf8::downgrade($self->[0]);
    } else {
	utf8::upgrade($self->[0]);
    }
    $self->[0];
}

package main;

# These tests are based on characters 128-255 not having latin1, and hence
# Unicode, semantics
# no feature "unicode_strings";

# Bug 34297
foreach my $t ("ASCII", "B" . uni_to_native("\366") . "se") {
    my $length = length $t;

    my $u = UTF8Toggle->new($t);
    is (length $u, $length, "length of '$t'");
    is (length $u, $length, "length of '$t'");
    is (length $u, $length, "length of '$t'");
    is (length $u, $length, "length of '$t'");
}

my $E_acute = uni_to_native("\311");
my $e_acute = uni_to_native("\351");
my $u = UTF8Toggle->new($E_acute);
my $lc = lc $u;
is (length $lc, 1);
is ($lc, $E_acute, "E acute -> e acute");
$lc = lc $u;
is (length $lc, 1);
is ($lc, $e_acute, "E acute -> e acute");
$lc = lc $u;
is (length $lc, 1);
is ($lc, $E_acute, "E acute -> e acute");

$u = UTF8Toggle->new($e_acute);
my $uc = uc $u;
is (length $uc, 1);
is ($uc, $e_acute, "e acute -> E acute");
$uc = uc $u;
is (length $uc, 1);
is ($uc, $E_acute, "e acute -> E acute");
$uc = uc $u;
is (length $uc, 1);
is ($uc, $e_acute, "e acute -> E acute");

$u = UTF8Toggle->new($E_acute);
$lc = lcfirst $u;
is (length $lc, 1);
is ($lc, $E_acute, "E acute -> e acute");
$lc = lcfirst $u;
is (length $lc, 1);
is ($lc, $e_acute, "E acute -> e acute");
$lc = lcfirst $u;
is (length $lc, 1);
is ($lc, $E_acute, "E acute -> e acute");

$u = UTF8Toggle->new($e_acute);
$uc = ucfirst $u;
is (length $uc, 1);
is ($uc, $e_acute, "e acute -> E acute");
$uc = ucfirst $u;
is (length $uc, 1);
is ($uc, $E_acute, "e acute -> E acute");
$uc = ucfirst $u;
is (length $uc, 1);
is ($uc, $e_acute, "e acute -> E acute");

my $have_setlocale = locales_enabled( [ 'LC_ALL', 'LC_CTYPE' ] );

SKIP: {
    if (!$have_setlocale) {
	skip "No setlocale", 24;
    } elsif (!setlocale(&POSIX::LC_ALL, "en_GB.ISO8859-1")) {
	skip "Could not setlocale to en_GB.ISO8859-1", 24;
    } elsif ($^O eq 'dec_osf' || $^O eq 'VMS') {
	skip "$^O has broken en_GB.ISO8859-1 locale", 24;
    } else {
        use locale;
	my $u = UTF8Toggle->new($E_acute);
	my $lc = lc $u;
	is (length $lc, 1);
	is ($lc, $e_acute, "E acute -> e acute");
	$lc = lc $u;
	is (length $lc, 1);
	is ($lc, $e_acute, "E acute -> e acute");
	$lc = lc $u;
	is (length $lc, 1);
	is ($lc, $e_acute, "E acute -> e acute");

	$u = UTF8Toggle->new($e_acute);
	my $uc = uc $u;
	is (length $uc, 1);
	is ($uc, $E_acute, "e acute -> E acute");
	$uc = uc $u;
	is (length $uc, 1);
	is ($uc, $E_acute, "e acute -> E acute");
	$uc = uc $u;
	is (length $uc, 1);
	is ($uc, $E_acute, "e acute -> E acute");

	$u = UTF8Toggle->new($E_acute);
	$lc = lcfirst $u;
	is (length $lc, 1);
	is ($lc, $e_acute, "E acute -> e acute");
	$lc = lcfirst $u;
	is (length $lc, 1);
	is ($lc, $e_acute, "E acute -> e acute");
	$lc = lcfirst $u;
	is (length $lc, 1);
	is ($lc, $e_acute, "E acute -> e acute");

	$u = UTF8Toggle->new($e_acute);
	$uc = ucfirst $u;
	is (length $uc, 1);
	is ($uc, $E_acute, "e acute -> E acute");
	$uc = ucfirst $u;
	is (length $uc, 1);
	is ($uc, $E_acute, "e acute -> E acute");
	$uc = ucfirst $u;
	is (length $uc, 1);
	is ($uc, $E_acute, "e acute -> E acute");
    }
}

my $tmpfile = tempfile();

foreach my $operator ('print', 'syswrite', 'syswrite len', 'syswrite off',
		      'syswrite len off') {
    foreach my $layer ('', $operator =~ /syswrite/ ? () : (':utf8')) {
	open my $fh, "+>:raw$layer", $tmpfile or die $!;
	my $pad = $operator =~ /\boff\b/ ? "\243" : "";
	my $trail = $operator =~ /\blen\b/ ? "!" : "";
	my $u = UTF8Toggle->new("$pad$E_acute\n$trail");
	my $l = UTF8Toggle->new("$pad$e_acute\n$trail", 1);
        no warnings 'deprecated';
	if ($operator eq 'print') {
	    no warnings 'utf8';
	    print $fh $u;
	    print $fh $u;
	    print $fh $u;
	    print $fh $l;
	    print $fh $l;
	    print $fh $l;
	} elsif ($operator eq 'syswrite') {
	    syswrite $fh, $u;
	    syswrite $fh, $u;
	    syswrite $fh, $u;
	    syswrite $fh, $l;
	    syswrite $fh, $l;
	    syswrite $fh, $l;
	} elsif ($operator eq 'syswrite len') {
	    syswrite $fh, $u, 2;
	    syswrite $fh, $u, 2;
	    syswrite $fh, $u, 2;
	    syswrite $fh, $l, 2;
	    syswrite $fh, $l, 2;
	    syswrite $fh, $l, 2;
	} elsif ($operator eq 'syswrite off'
		 || $operator eq 'syswrite len off') {
	    syswrite $fh, $u, 2, 1;
	    syswrite $fh, $u, 2, 1;
	    syswrite $fh, $u, 2, 1;
	    syswrite $fh, $l, 2, 1;
	    syswrite $fh, $l, 2, 1;
	    syswrite $fh, $l, 2, 1;
	} else {
	    die $operator;
	}

	seek $fh, 0, 0 or die $!;
	my $line;
	chomp ($line = <$fh>);
	is ($line, $E_acute, "$operator $layer");
	chomp ($line = <$fh>);
	is ($line, $E_acute, "$operator $layer");
	chomp ($line = <$fh>);
	is ($line, $E_acute, "$operator $layer");
	chomp ($line = <$fh>);
	is ($line, $e_acute, "$operator $layer");
	chomp ($line = <$fh>);
	is ($line, $e_acute, "$operator $layer");
	chomp ($line = <$fh>);
	is ($line, $e_acute, "$operator $layer");

	close $fh or die $!;
    }
}

my $little = "\243\243";
my $big = " \243 $little ! $little ! $little \243 ";
my $right = rindex $big, $little;
my $right1 = rindex $big, $little, 11;
my $left = index $big, $little;
my $left1 = index $big, $little, 4;

cmp_ok ($right, ">", $right1, "Sanity check our rindex tests");
cmp_ok ($left, "<", $left1, "Sanity check our index tests");

foreach my $b ($big, UTF8Toggle->new($big)) {
    foreach my $l ($little, UTF8Toggle->new($little),
		   UTF8Toggle->new($little, 1)) {
	is (rindex ($b, $l), $right, "rindex");
	is (rindex ($b, $l), $right, "rindex");
	is (rindex ($b, $l), $right, "rindex");

	is (rindex ($b, $l, 11), $right1, "rindex 11");
	is (rindex ($b, $l, 11), $right1, "rindex 11");
	is (rindex ($b, $l, 11), $right1, "rindex 11");

	is (index ($b, $l), $left, "index");
	is (index ($b, $l), $left, "index");
	is (index ($b, $l), $left, "index");

	is (index ($b, $l, 4), $left1, "index 4");
	is (index ($b, $l, 4), $left1, "index 4");
	is (index ($b, $l, 4), $left1, "index 4");
    }
}

my $bits = $E_acute;
foreach my $pieces ($bits, UTF8Toggle->new($bits)) {
    like ($bits ^ $pieces, qr/\A\0+\z/, "something xor itself is zeros");
    like ($bits ^ $pieces, qr/\A\0+\z/, "something xor itself is zeros");
    like ($bits ^ $pieces, qr/\A\0+\z/, "something xor itself is zeros");

    like ($pieces ^ $bits, qr/\A\0+\z/, "something xor itself is zeros");
    like ($pieces ^ $bits, qr/\A\0+\z/, "something xor itself is zeros");
    like ($pieces ^ $bits, qr/\A\0+\z/, "something xor itself is zeros");
}

foreach my $value ("\243", UTF8Toggle->new("\243")) {
    is (pack ("A/A", $value), pack ("A/A", "\243"),
	"pack copes with overloading");
    is (pack ("A/A", $value), pack ("A/A", "\243"));
    is (pack ("A/A", $value), pack ("A/A", "\243"));
}

foreach my $value ("\243", UTF8Toggle->new("\243")) {
    my $v;
    $v = substr $value, 0, 1;
    is ($v, "\243");
    $v = substr $value, 0, 1;
    is ($v, "\243");
    $v = substr $value, 0, 1;
    is ($v, "\243");
}

{
    package RT69422;
    use overload '""' => sub { $_[0]->{data} }
}

{
    my $text = bless { data => "\x{3075}" }, 'RT69422';
    my $p = substr $text, 0, 1;
    is ($p, "\x{3075}");
}

TODO: {
    local $::TODO = 'RT #3054: Recursive operator overloading overflows the C stack';
    # XXX this test is expected to SEGV, and can produce
    #    sh: line 1:  5106 Segmentation fault
    # on STDERR. So just completely disable for now
    todo_skip($::TODO);
    fresh_perl_is(<<'EOP', "ok\n", {}, 'RT #3054: Recursive operator overloading should not crash the interpreter');
    use overload '""' => sub { "$_[0]" };
    print bless {}, __PACKAGE__;
    print "ok\n";
EOP
}

TODO: {
    local $::TODO = 'RT #3270: Overloaded operators can not be treated as lvalues';
    fresh_perl_is(<<'EOP', '', {stderr => 1}, 'RT #3270: Overloaded operator that returns an lvalue can be used as an lvalue');
    use overload '.' => \&dot;
    sub dot : lvalue {my ($obj, $method) = @_; $obj -> {$method};}
    my $o  = bless {} => "main";
    $o.foo = "bar";
EOP
}
