#!./perl -w

#
# Verify which OP= operators warn if their targets are undefined.
# Based on redef.t, contributed by Graham Barr <Graham.Barr@tiuk.ti.com>
#	-- Robin Barker 
#
# Now almost completely rewritten.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

my (%should_warn, %should_not);
++$should_warn{$_} foreach qw(* / x & ** << >>);
++$should_not{$_} foreach qw(+ - . | ^ && ||);

my %integer;
$integer{$_} = 0 foreach qw(* / % + -);

sub TIESCALAR { my $x; bless \$x }
sub FETCH { ${$_[0]} }
sub STORE { ${$_[0]} = $_[1] }

sub test_op {
    my ($tie, $int, $op_seq, $warn) = @_;
    my $code = "sub {\n";
    $code .= "use integer;" if $int;
    $code .= "my \$x;\n";
    $code .= "tie \$x, 'main';\n" if $tie;
    $code .= "$op_seq;\n}\n";

    my $sub = eval $code;
    is($@, '', "Can eval code for $op_seq");
    if ($warn) {
	warning_like($sub, qr/^Use of uninitialized value/,
		     "$op_seq$tie$int warns");
    } else {
	warning_is($sub, undef, "$op_seq$tie$int does not warn");
    }
}

# go through all tests once normally and once with tied $x
for my $tie ("", ", tied") {
    foreach my $integer ('', ', int') {
	test_op($tie, $integer, $_, 0) foreach qw($x++ $x-- ++$x --$x);
    }

    foreach (keys %should_warn, keys %should_not) {
	test_op($tie, '', "\$x $_= 1", $should_warn{$_});
	next unless exists $integer{$_};
	test_op($tie, ', int', "\$x $_= 1", $should_warn{$_});
    }

    foreach (qw(| ^ &)) {
	test_op($tie, '', "\$x $_= 'x'", $should_warn{$_});
    }
}

done_testing();
