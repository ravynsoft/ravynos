#!./perl -w

$|=1;

BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/ && $Config{'osname'} ne 'VMS') {
        print "1..0\n";
        exit 0;
    }
}

use strict;
use Test::More;

{
    my @warnings;

    BEGIN {
	local $SIG{__WARN__} = sub {
	    push @warnings, "@_";
	};

	use_ok('Opcode', qw(
	opcodes opdesc opmask verify_opset
	opset opset_to_ops opset_to_hex invert_opset
	opmask_add full_opset empty_opset define_optag
			   ));
    }

    is_deeply(\@warnings, [], "No warnings loading Opcode");
}

# --- opset_to_ops and opset

my @empty_l = opset_to_ops(empty_opset);
is_deeply (\@empty_l, []);

my @full_l1  = opset_to_ops(full_opset);
is (scalar @full_l1, scalar opcodes());

{
    local $::TODO = "opcodes in list context not yet implemented";
    my @full_l2 = eval {opcodes()};
    is($@, '');
    is_deeply(\@full_l1, \@full_l2);
}

@empty_l = opset_to_ops(opset(':none'));
is_deeply(\@empty_l, []);

my @full_l3 = opset_to_ops(opset(':all'));
is_deeply(\@full_l1, \@full_l3);

my $s1 = opset(      'padsv');
my $s2 = opset($s1,  'padav');
my $s3 = opset($s2, '!padav');
isnt($s1, $s2);
is($s1, $s3);

# --- define_optag

is(eval { opset(':_tst_') }, undef);
like($@, qr/Unknown operator tag ":_tst_"/);
define_optag(":_tst_", opset(qw(padsv padav padhv)));
isnt(eval { opset(':_tst_') }, undef);
is($@, '');

# --- opdesc and opcodes

is(opdesc("gv"), "glob value");
my @desc = opdesc(':_tst_','stub');
is_deeply(\@desc, ['private variable', 'private array', 'private hash', 'stub']);
isnt(opcodes(), 0);

# --- invert_opset

$s1 = opset(qw(fileno padsv padav));
my @o1 = opset_to_ops(invert_opset($s1));
is(scalar @o1, opcodes-3);

# --- opmask

is(opmask(), empty_opset());
is(length opmask(), int((opcodes()+7)/8));

# --- verify_opset

is(verify_opset($s1), 1);
is(verify_opset(42), 0);

# --- opmask_add

opmask_add(opset(qw(fileno)));	# add to global op_mask
is(eval 'fileno STDOUT', undef);
like($@, qr/'fileno' trapped/);

# --- check use of bit vector ops on opsets

$s1 = opset('padsv');
$s2 = opset('padav');
$s3 = opset('padsv', 'padav', 'padhv');

# Non-negated
is(($s1 | $s2), opset($s1,$s2));
is(($s2 & $s3), opset($s2));
is(($s2 ^ $s3), opset('padsv','padhv'));

# Negated, e.g., with possible extra bits in last byte beyond last op bit.
# The extra bits mean we can't just say ~mask eq invert_opset(mask).

@o1 = opset_to_ops(           ~ $s3);
my @o2 = opset_to_ops(invert_opset $s3);
is_deeply(\@o1, \@o2);

# --- test context of undocumented _safe_call_sv (used by Safe.pm)

my %inc = %INC;
my $expect;
sub f {
    %INC = %inc;
    no warnings 'uninitialized';
    is wantarray, $expect,
       sprintf "_safe_call_sv gives %s context",
		qw[void scalar list][$expect + defined $expect]
};
Opcode::_safe_call_sv("main", empty_opset, \&f);
$expect = !1;
$_ = Opcode::_safe_call_sv("main", empty_opset, \&f);
$expect = !0;
() = Opcode::_safe_call_sv("main", empty_opset, \&f);

# --- finally, check some opname assertions

foreach my $opname (@full_l1) {
    unlike($opname, qr/\W/, "opname $opname has no non-'word' characters");
    unlike($opname, qr/^\d/, "opname $opname does not start with a digit");
}

done_testing();
