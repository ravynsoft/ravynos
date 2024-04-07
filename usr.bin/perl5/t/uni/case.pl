BEGIN {
    require "./test.pl";
    set_up_inc(qw(../lib .));
    skip_all_without_unicode_tables();
}
use strict;
use warnings;
use feature 'unicode_strings';

sub unidump {
    join "", map { sprintf "\\x{%04X}", $_ } unpack "W*", $_[0];
}

sub casetest {
    my ($already_run, $base, %funcs) = @_;

    my %spec;

    # For each provided function run it, and run a version with some extra
    # characters afterwards. Use a recycling symbol, as it doesn't change case.
    # $already_run is the number of extra tests the caller has run before this
    # call.
    my $ballast = chr (0x2672) x 3;
    foreach my $name (keys %funcs) {
        $funcs{"${name}_with_ballast"} =
		   sub {my $r = $funcs{$name}->($_[0] . $ballast); # Add it before
			$r =~ s/$ballast\z//so # Remove it afterwards
			    or die "'$_[0]' to '$r' mangled";
			$r; # Result with $ballast removed.
		    };
    }

    use Unicode::UCD 'prop_invmap';

    # Get the case mappings
    my ($invlist_ref, $invmap_ref, undef, $default) = prop_invmap($base);
    my %simple;

    for my $i (0 .. @$invlist_ref - 1 - 1) {
        next if $invmap_ref->[$i] == $default;

        # Add simple mappings to the simples test list
        if (! ref $invmap_ref->[$i]) {

            # The returned map needs to have adjustments made.  Each
            # subsequent element of the range requires adjustment of +1 from
            # the previous element
            my $adjust = 0;
            for my $k ($invlist_ref->[$i] .. $invlist_ref->[$i+1] - 1) {
                $simple{$k} = $invmap_ref->[$i] + $adjust++;
            }
        }
        else {  # The return is a list of the characters mapped-to.
                # prop_invmap() guarantees a single element in the range in
                # this case, so no adjustments are needed.
            $spec{$invlist_ref->[$i]} = pack "W*" , @{$invmap_ref->[$i]};
        }
    }

    my %seen;

    for my $i (sort keys %simple) {
	$seen{$i}++;
    }
    print "# ", scalar keys %simple, " simple mappings\n";

    for my $i (sort keys %spec) {
	if (++$seen{$i} == 2) {
	    warn sprintf "$base: $i seen twice\n";
	}
    }
    print "# ", scalar keys %spec, " special mappings\n";

    my %none;
    for my $i (map { ord } split //,
	       "\e !\"#\$%&'()+,-./0123456789:;<=>?\@[\\]^_{|}~\b") {
	next if pack("W", $i) =~ /\w/;
	$none{$i}++ unless $seen{$i};
    }
    print "# ", scalar keys %none, " noncase mappings\n";


    my $test = $already_run + 1;

    for my $ord (sort { $a <=> $b } keys %simple) {
	my $char = pack "W", $ord;
        my $disp_input = unidump($char);

        my $expected = pack("W", $simple{$ord});
        my $disp_expected = unidump($expected);

	foreach my $name (sort keys %funcs) {
	    my $got = $funcs{$name}->($char);
	    is( $got, $expected,
               "Verify $name(\"$disp_input\") eq \"$disp_expected\"");
	}
    }

    for my $ord (sort { $a <=> $b } keys %spec) {
	my $char = pack "W", $ord;
        my $disp_input = unidump($char);

	my $expected = unidump($spec{$ord});

	foreach my $name (sort keys %funcs) {
	    my $got = $funcs{$name}->($char);
            is( unidump($got), $expected,
               "Verify $name(\"$disp_input\") eq \"$expected\"");
	}
    }

    for my $ord (sort { $a <=> $b } keys %none) {
	my $char = pack "W", $ord;
        my $disp_input = unidump($char);

	foreach my $name (sort keys %funcs) {
	    my $got = $funcs{$name}->($char);
            is( $got, $char,
               "Verify $name(\"$disp_input\") eq \"$disp_input\"");
	}
    }

    plan $already_run +
	((scalar keys %simple) +
	 (scalar keys %spec) +
	 (scalar keys %none)) * scalar keys %funcs;
}

1;
