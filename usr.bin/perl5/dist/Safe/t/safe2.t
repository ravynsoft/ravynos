#!./perl -w
$|=1;
BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/ && $Config{'osname'} ne 'VMS') {
        print "1..0\n";
        exit 0;
    }
}

# Tests Todo:
#	'main' as root

use Opcode 1.00, qw(opdesc opset opset_to_ops opset_to_hex
	opmask_add full_opset empty_opset opcodes opmask define_optag);

use Safe 1.00;

use Test::More;
my $TB = Test::Builder->new();

# Set up a package namespace of things to be visible to the unsafe code
$Root::foo = "visible";
our $bar = "invisible";

# Stop perl from moaning about identifies which are apparently only used once
$Root::foo .= "";

my $cpt;
# create and destroy a couple of automatic Safe compartments first
$cpt = new Safe or die;
$cpt = new Safe or die;

$cpt = new Safe "Root";

$cpt->permit(qw(:base_io));

$cpt->reval(q{ system("echo not ok 1"); });
like($@, qr/^'?system'? trapped by operation mask/);

$cpt->reval(q{
    print $foo eq 'visible'		? "ok 2\n" : "not ok 2\n";
    print $main::foo  eq 'visible'	? "ok 3\n" : "not ok 3\n";
    print defined($bar)			? "not ok 4\n" : "ok 4\n";
    print defined($::bar)		? "not ok 5\n" : "ok 5\n";
    print defined($main::bar)		? "not ok 6\n" : "ok 6\n";
});
$TB->current_test(6);
is($@, '');

$foo = "ok 8\n";
%bar = (key => "ok 9\n");
@baz = (); push(@baz, "o", "10");
$glob = "ok 11\n";
@glob = qw(not ok 16);

sub sayok { print "ok @_\n" }

$cpt->share(qw($foo %bar @baz *glob sayok));
$cpt->share('$"') unless $Config{use5005threads};

{
    $" = 'k ';
    $cpt->reval(q{
    package other;
    sub other_sayok { print "ok @_\n" }
    package main;
    print $foo ? $foo : "not ok 8\n";
    print $bar{key} ? $bar{key} : "not ok 9\n";
    (@baz) ? print "@baz\n" : print "not ok 10\n";
    print $glob;
    other::other_sayok(12);
    $foo =~ s/8/14/;
    $bar{new} = "ok 15\n";
    @glob = qw(ok 16);
    $" = ' ';
});
}
$TB->current_test(12);
is($@, '');
is($foo, "ok 14\n");
is($bar{new}, "ok 15\n");
is("@glob", "ok 16");

$Root::foo = "not ok 17";
@{$cpt->varglob('bar')} = qw(not ok 18);
${$cpt->varglob('foo')} = "ok 17";
@Root::bar = "ok";
push(@Root::bar, "18"); # Two steps to prevent "Identifier used only once..."

is($Root::foo, 'ok 17');
is("@{$cpt->varglob('bar')}", 'ok 18');

use strict;

my $m1 = $cpt->mask;
$cpt->trap("negate");
my $m2 = $cpt->mask;
my @masked = opset_to_ops($m1);
is(opset("negate", @masked), $m2);

is(eval { $cpt->mask("a bad mask") }, undef);
isnt($@, '');

is($cpt->reval("2 + 2"), 4);

my $test = $TB->current_test() + 1;
$cpt->reval("
    my \$todo = \$] < 5.021007 ? ' # TODO' : '';
    print defined wantarray
	? qq'not ok $test\$todo\n'
	: qq'ok $test\$todo\n'
");
++$test;
my $t_scalar = $cpt->reval("print wantarray ? 'not ok $test\n' : 'ok $test\n'");
++$test;
my @t_array  = $cpt->reval("print wantarray ? 'ok $test\n' : 'not ok $test\n'; (2,3,4)");
$TB->current_test($test);

is($t_array[2], 4);

is($cpt->reval('@ary=(6,7,8);@ary'), 3);

my $t_scalar2 = $cpt->reval('die "foo bar"; 1');
is($t_scalar2, undef);
like($@, qr/foo bar/);

# --- rdo
  
$! = 0;
my $nosuch = '/non/existent/file.name';
open(NOSUCH, '<', $nosuch);
if ($@) {
    my $errno = $!;
    die "Eek! Attempting to open $nosuch failed, but \$! is still 0" unless $!;
    $! = 0;
    $cpt->rdo($nosuch);
    is($!, $errno);
} else {
    die "Eek! Didn't expect $nosuch to be there.";
}
close(NOSUCH);

done_testing();
