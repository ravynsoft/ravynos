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

package test;	# test from somewhere other than main

our $bar;

use Opcode 1.00, qw(opdesc opset opset_to_ops opset_to_hex
	opmask_add full_opset empty_opset opcodes opmask define_optag);

use Safe 1.00;
use Test::More;

my $cpt;
# create and destroy some automatic Safe compartments first
$cpt = new Safe or die;
$cpt = new Safe or die;
$cpt = new Safe or die;

$cpt = new Safe "Root" or die;

foreach(1..3) {
	$foo = 42;

	$cpt->share(qw($foo));

	is(${$cpt->varglob('foo')}, 42);

	${$cpt->varglob('foo')} = 9;

	is($foo, 9);

	is($cpt->reval('$foo'), 9);
	is($cpt->reval('$::foo'), 9, "check 'main' has been changed");
	is($cpt->reval('$main::foo'), 9, "check 'main' has been changed");
	is($cpt->reval('$test::foo'), undef,
	   "check we can't see our test package");
	is($cpt->reval('${"test::foo"}'), undef,
	   "check we can't see our test package");

	$cpt->erase;
	is($cpt->reval('$foo'), undef,
	   'erasing the compartment deleted all variables');

	# Note that we *must* use $cpt->varglob here because if we used
	# $Root::foo etc we would still see the original values!
	# This seems to be because the compiler has created an extra ref.

	is(${$cpt->varglob('foo')}, undef);
}

done_testing();
