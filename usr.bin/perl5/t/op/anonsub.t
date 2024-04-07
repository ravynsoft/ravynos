#!./perl -w

chdir 't' if -d 't';
require './test.pl';
set_up_inc('../lib');

use strict;

$|=1;

run_multiple_progs('', \*DATA);

foreach my $code ('sub;', 'sub ($) ;', '{ $x = sub }', 'sub ($) && 1') {
    eval $code;
    like($@, qr/^Illegal declaration of anonymous subroutine at/,
	 "'$code' is illegal");
}

{
    local $::TODO;
    $::TODO = 'RT #17589 not completely resolved';
    # Here's a patch. It makes "sub;" and similar report an error immediately
    # from the lexer. However the solution is not complete, it doesn't
    # handle the case "sub ($) : lvalue;" (marked as a TODO test), because
    # it's handled by the lexer in separate tokens, hence more difficult to
    # work out.
    my $code = 'sub ($) : lvalue;';
    eval $code;
    like($@, qr/^Illegal declaration of anonymous subroutine at/,
	 "'$code' is illegal");
}

eval "sub #foo\n{print 1}";
is($@, '');

done_testing();

__END__
sub X {
    my $n = "ok 1\n";
    sub { print $n };
}
my $x = X();
undef &X;
$x->();
EXPECT
ok 1
########
sub X {
    my $n = "ok 1\n";
    sub {
        my $dummy = $n;	# eval can't close on $n without internal reference
	eval 'print $n';
	die $@ if $@;
    };
}
my $x = X();
undef &X;
$x->();
EXPECT
ok 1
########
sub X {
    my $n = "ok 1\n";
    eval 'sub { print $n }';
}
my $x = X();
die $@ if $@;
undef &X;
$x->();
EXPECT
ok 1
########
sub X;
sub X {
    my $n = "ok 1\n";
    eval 'sub Y { my $p = shift; $p->() }';
    die $@ if $@;
    Y(sub { print $n });
}
X();
EXPECT
ok 1
########
print sub { return "ok 1\n" } -> ();
EXPECT
ok 1
########
my @void_warnings;
{
    use warnings;
    local $SIG{'__WARN__'} = sub { push @void_warnings, @_ };
    sub { 1 };
    1
}
"@void_warnings"
EXPECT
Useless use of anonymous subroutine in void context at - line 5.
########
# [perl #71154] undef &$code makes $code->() die with: Not a CODE reference
sub __ANON__ { print "42\n" }
undef &{$x=sub{}};
$x->();
EXPECT
Undefined subroutine called at - line 4.
########
# NAME anon constant clobbering __ANON__
sub __ANON__ { "42\n" }
print __ANON__;
sub(){3};
EXPECT
42
########
# NAME undef &anon giving it a freed GV
$_ = sub{};
delete $::{__ANON__};
undef &$_; # SvREFCNT_dec + inc on a GV with a refcnt of 1
           # so now SvTYPE(CvGV(anon)) is 0xff == freed
if (!eval { require B }) { # miniperl, presumably
    print "__ANON__\n";
} else {
    print B::svref_2object($_)->GV->NAME, "\n";
}
EXPECT
__ANON__
