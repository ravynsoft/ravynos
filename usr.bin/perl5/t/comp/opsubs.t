#!./perl -w

# Uncomment this for testing, but don't leave it in for "production", as
# we've not yet verified that use works.
# use strict;

$|++;

print "1..36\n";
my $test = 0;

sub failed {
    my ($got, $expected, $name) = @_;

    if ($::TODO) {
	print "not ok $test - $name # TODO: $::TODO\n";
    }
    else {
	print "not ok $test - $name\n";
    }
    my @caller = caller(1);
    print "# Failed test at $caller[1] line $caller[2]\n";
    if (defined $got) {
	print "# Got '$got'\n";
    } else {
	print "# Got undef\n";
    }
    print "# Expected $expected\n";
    return;
}

sub like {
    my ($got, $pattern, $name) = @_;
    $test = $test + 1;
    if (defined $got && $got =~ $pattern) {
	if ($::TODO) {
	    print "ok $test - $name # TODO: $::TODO\n";
	}
	else {
	    print "ok $test - $name\n";
	}
	# Principle of least surprise - maintain the expected interface, even
	# though we aren't using it here (yet).
	return 1;
    }
    failed($got, $pattern, $name);
}

sub is {
    my ($got, $expect, $name) = @_;
    $test = $test + 1;
    if (defined $got && $got eq $expect) {
	if ($::TODO) {
	    print "ok $test - $name # TODO: $::TODO\n";
	}
	else {
	    print "ok $test - $name\n";
	}
	return 1;
    }
    failed($got, "'$expect'", $name);
}

sub isnt {
    my ($got, $expect, $name) = @_;
    $test = $test + 1;
    if (defined $got && $got ne $expect) {
	if ($::TODO) {
	    print "ok $test - $name # TODO: $::TODO\n";
	}
	else {
	    print "ok $test - $name\n";
	}
	return 1;
    }
    failed($got, "not '$expect'", $name);
}

sub can_ok {
    my ($class, $method) = @_;
    $test = $test + 1;
    if (eval { $class->can($method) }) {
	if ($::TODO) {
	    print "ok $test - $class->can('$method') # TODO: $::TODO\n";
	}
	else {
	    print "ok $test - $class->can('$method')\n";
	}
	return 1;
    }
    my @caller = caller;
    print "# Failed test at $caller[1] line $caller[2]\n";
    print "# $class cannot $method\n";
    return;
}

=pod

Even if you have a C<sub q{}>, calling C<q()> will be parsed as the
C<q()> operator.  Calling C<&q()> or C<main::q()> gets you the function.
This test verifies this behavior for nine different operators.

=cut

sub m  { return "m-".shift }
sub q  { return "q-".shift }
sub qq { return "qq-".shift }
sub qr { return "qr-".shift }
sub qw { return "qw-".shift }
sub qx { return "qx-".shift }
sub s  { return "s-".shift }
sub tr { return "tr-".shift }
sub y  { return "y-".shift }

# m operator
can_ok( 'main', "m" );
SILENCE_WARNING: { # Complains because $_ is undef
    local $^W;		       
    isnt( m('unqualified'), "m-unqualified", "m('unqualified') is oper" );
}
is( main::m('main'), "m-main", "main::m() is func" );
is( &m('amper'), "m-amper", "&m() is func" );

# q operator
can_ok( 'main', "q" );
isnt( q('unqualified'), "q-unqualified", "q('unqualified') is oper" );
is( main::q('main'), "q-main", "main::q() is func" );
is( &q('amper'), "q-amper", "&q() is func" );

# qq operator
can_ok( 'main', "qq" );
isnt( qq('unqualified'), "qq-unqualified", "qq('unqualified') is oper" );
is( main::qq('main'), "qq-main", "main::qq() is func" );
is( &qq('amper'), "qq-amper", "&qq() is func" );

# qr operator
can_ok( 'main', "qr" );
isnt( qr('unqualified'), "qr-unqualified", "qr('unqualified') is oper" );
is( main::qr('main'), "qr-main", "main::qr() is func" );
is( &qr('amper'), "qr-amper", "&qr() is func" );

# qw operator
can_ok( 'main', "qw" );
isnt( qw('unqualified'), "qw-unqualified", "qw('unqualified') is oper" );
is( main::qw('main'), "qw-main", "main::qw() is func" );
is( &qw('amper'), "qw-amper", "&qw() is func" );

# qx operator
can_ok( 'main', "qx" );
eval q{
    BEGIN {
        *CORE::GLOBAL::readpipe = sub { die "readpipe called" };
    }
    qx('unqualified');
};
like( $@, qr/^readpipe called/, "qx('unqualified') is oper" );
is( main::qx('main'), "qx-main", "main::qx() is func" );
is( &qx('amper'), "qx-amper", "&qx() is func" );

# s operator
can_ok( 'main', "s" );
eval "s('unqualified')";
like( $@, qr/^Substitution replacement not terminated/, "s('unqualified') doesn't work" );
is( main::s('main'), "s-main", "main::s() is func" );
is( &s('amper'), "s-amper", "&s() is func" );

# tr operator
can_ok( 'main', "tr" );
eval "tr('unqualified')";
like( $@, qr/^Transliteration replacement not terminated/, "tr('unqualified') doesn't work" );
is( main::tr('main'), "tr-main", "main::tr() is func" );
is( &tr('amper'), "tr-amper", "&tr() is func" );

# y operator
can_ok( 'main', "y" );
eval "y('unqualified')";
like( $@, qr/^Transliteration replacement not terminated/, "y('unqualified') doesn't work" );
is( main::y('main'), "y-main", "main::y() is func" );
is( &y('amper'), "y-amper", "&y() is func" );

=pod

from irc://irc.perl.org/p5p 2004/08/12

 <kane-xs>  bug or feature?
 <purl>     You decide!!!!
 <kane-xs>  [kane@coke ~]$ perlc -le'sub y{1};y(1)'
 <kane-xs>  Transliteration replacement not terminated at -e line 1.
 <Nicholas> bug I think
 <kane-xs>  i'll perlbug
 <rgs>      feature
 <kane-xs>  smiles at rgs
 <kane-xs>  done
 <rgs>      will be closed at not a bug,
 <rgs>      like the previous reports of this one
 <Nicholas> feature being first class and second class keywords?
 <rgs>      you have similar ones with q, qq, qr, qx, tr, s and m
 <rgs>      one could say 1st class keywords, yes
 <rgs>      and I forgot qw
 <kane-xs>  hmm silly...
 <Nicholas> it's acutally operators, isn't it?
 <Nicholas> as in you can't call a subroutine with the same name as an
            operator unless you have the & ?
 <kane-xs>  or fqpn (fully qualified package name)
 <kane-xs>  main::y() works just fine
 <kane-xs>  as does &y; but not y()
 <Andy>     If that's a feature, then let's write a test that it continues
            to work like that.

=cut
