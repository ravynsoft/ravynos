#
# t/test.pl - most of Test::More functionality without the fuss


# NOTE:
#
# Do not rely on features found only in more modern Perls here, as some CPAN
# distributions copy this file and must operate on older Perls. Similarly, keep
# things, simple as this may be run under fairly broken circumstances. For
# example, increment ($x++) has a certain amount of cleverness for things like
#
#   $x = 'zz';
#   $x++; # $x eq 'aaa';
#
# This stands more chance of breaking than just a simple
#
#   $x = $x + 1
#
# In this file, we use the latter "Baby Perl" approach, and increment
# will be worked over by t/op/inc.t

$| = 1;
$Level = 1;
my $test = 1;
my $planned;
my $noplan;

# Fatalize warnings, so that we don't introduce new warnings.  But on early
# perls the burden of avoiding warnings becomes too large, and someone still
# trying to use such outmoded versions should be willing to accept warnings in
# our test suite.
$SIG{__WARN__} = sub { die "Fatalized: $_[0]" } if $] ge "5.6.0";

# This defines ASCII/UTF-8 vs EBCDIC/UTF-EBCDIC
$::IS_ASCII  = ord 'A' ==  65;

$TODO = 0;
$NO_ENDING = 0;
$Tests_Are_Passing = 1;

# Use this instead of print to avoid interference while testing globals.
sub _print {
    local($\, $", $,) = (undef, ' ', '') if "$]" >= 5.004;
    print STDOUT @_;
}

sub _print_stderr {
    local($\, $", $,) = (undef, ' ', '') if "$]" >= 5.004;
    print STDERR @_;
}

sub plan {
    my $n;
    if (@_ == 1) {
	$n = shift;
	if ($n eq 'no_plan') {
	  undef $n;
	  $noplan = 1;
	}
    } else {
	my %plan = @_;
	$plan{skip_all} and skip_all($plan{skip_all});
	$n = $plan{tests};
    }
    _print "1..$n\n" unless $noplan;
    $planned = $n;
}


# Set the plan at the end.  See Test::More::done_testing.
sub done_testing {
    my $n = $test - 1;
    $n = shift if @_;

    _print "1..$n\n";
    $planned = $n;
}


END {
    my $ran = $test - 1;
    if (!$NO_ENDING) {
	if (defined $planned && $planned != $ran) {
	    _print_stderr
		"# Looks like you planned $planned tests but ran $ran.\n";
	} elsif ($noplan) {
	    _print "1..$ran\n";
	}
    }
}

sub _diag {
    return unless @_;
    my @mess = _comment(@_);
    $TODO ? _print(@mess) : _print_stderr(@mess);
}

# Use this instead of "print STDERR" when outputting failure diagnostic
# messages
sub diag {
    _diag(@_);
}

# Use this instead of "print" when outputting informational messages
sub note {
    return unless @_;
    _print( _comment(@_) );
}

sub _comment {
    return map { /^#/ ? "$_\n" : "# $_\n" }
           map { split /\n/ } @_;
}

sub _have_dynamic_extension {
    my $extension = shift;
    unless (eval {require Config; 1}) {
	warn "test.pl had problems loading Config: $@";
	return 1;
    }
    $extension =~ s!::!/!g;
    return 1 if ($Config::Config{extensions} =~ /\b$extension\b/);
}

sub skip_all {
    if (@_) {
        _print "1..0 # Skip @_\n";
    } else {
	_print "1..0\n";
    }
    exit(0);
}

sub BAIL_OUT {
    my ($reason) = @_;
    _print("Bail out!  $reason\n");
    exit 255;
}

sub _ok {
    my ($pass, $where, $name, @mess) = @_;
    # Do not try to microoptimize by factoring out the "not ".
    # VMS will avenge.
    my $out;
    if ($name) {
        # escape out '#' or it will interfere with '# skip' and such
        $name =~ s/#/\\#/g;
	$out = $pass ? "ok $test - $name" : "not ok $test - $name";
    } else {
	$out = $pass ? "ok $test" : "not ok $test";
    }

    if ($TODO) {
	$out = $out . " # TODO $TODO";
    } else {
	$Tests_Are_Passing = 0 unless $pass;
    }

    _print "$out\n";

    if ($pass) {
	note @mess; # Ensure that the message is properly escaped.
    }
    else {
	my $msg = "# Failed test $test - ";
	$msg.= "$name " if $name;
	$msg .= "$where\n";
	_diag $msg;
	_diag @mess;
    }

    $test = $test + 1; # don't use ++

    return $pass;
}

sub _where {
    my @caller = caller($Level);
    return "at $caller[1] line $caller[2]";
}

sub ok ($@) {
    my ($pass, $name, @mess) = @_;
    _ok($pass, _where(), $name, @mess);
}

sub _q {
    my $x = shift;
    return 'undef' unless defined $x;
    my $q = $x;
    $q =~ s/\\/\\\\/g;
    $q =~ s/'/\\'/g;
    return "'$q'";
}

sub _qq {
    my $x = shift;
    return defined $x ? '"' . display ($x) . '"' : 'undef';
};

# Support pre-5.10 Perls, for the benefit of CPAN dists that copy this file.
# Note that chr(90) exists in both ASCII ("Z") and EBCDIC ("!").
my $chars_template = defined(eval { pack "W*", 90 }) ? "W*" : defined(eval { pack "U*", 90 }) ? "U*" : "C*";
eval 'sub re::is_regexp { ref($_[0]) eq "Regexp" }'
    if !defined &re::is_regexp;

# keys are the codes \n etc map to, values are 2 char strings such as \n
my %backslash_escape;
my $x;
foreach $x (split //, 'nrtfa\\\'"') {
    $backslash_escape{ord eval "\"\\$x\""} = "\\$x";
}
# A way to display scalars containing control characters and Unicode.
# Trying to avoid setting $_, or relying on local $_ to work.
sub display {
    my @result;
    my $x;
    foreach $x (@_) {
        if (defined $x and not ref $x) {
            my $y = '';
            my $c;
            foreach $c (unpack($chars_template, $x)) {
                if ($c > 255) {
                    $y = $y . sprintf "\\x{%x}", $c;
                } elsif ($backslash_escape{$c}) {
                    $y = $y . $backslash_escape{$c};
                } elsif ($c < ord " ") {
                    # Use octal for characters with small ordinals that are
                    # traditionally expressed as octal: the controls below
                    # space, which on EBCDIC are almost all the controls, but
                    # on ASCII don't include DEL nor the C1 controls.
                    $y = $y . sprintf "\\%03o", $c;
                } elsif ($::IS_ASCII && $c <= ord('~')) {
                    $y = $y . chr $c;
                } elsif ( ! $::IS_ASCII
                         && eval 'chr $c =~ /[^[:^print:][:^ascii:]]/')
                        # The pattern above is equivalent (by de Morgan's
                        # laws) to:
                        #     $z =~ /(?[ [:print:] & [:ascii:] ])/
                        # or, $z is an ascii printable character
                        # The /a modifier doesn't go back so far.
                {
                    $y = $y . chr $c;
                }
                elsif ($@) { # Should only be an error on platforms too
                             # early to have the [:posix:] syntax, which
                             # also should be ASCII ones
                    die __FILE__ . __LINE__
                      . ": Unexpected non-ASCII platform; $@";
                }
                else {
                    $y = $y . sprintf "\\x%02X", $c;
                }
            }
            $x = $y;
        }
        return $x unless wantarray;
        push @result, $x;
    }
    return @result;
}

sub is ($$@) {
    my ($got, $expected, $name, @mess) = @_;

    my $pass;
    if( !defined $got || !defined $expected ) {
        # undef only matches undef
        $pass = !defined $got && !defined $expected;
    }
    else {
        $pass = $got eq $expected;
    }

    unless ($pass) {
	unshift(@mess, "#      got "._qq($got)."\n",
		       "# expected "._qq($expected)."\n");
    }
    _ok($pass, _where(), $name, @mess);
}

sub isnt ($$@) {
    my ($got, $isnt, $name, @mess) = @_;

    my $pass;
    if( !defined $got || !defined $isnt ) {
        # undef only matches undef
        $pass = defined $got || defined $isnt;
    }
    else {
        $pass = $got ne $isnt;
    }

    unless( $pass ) {
        unshift(@mess, "# it should not be "._qq($got)."\n",
                       "# but it is.\n");
    }
    _ok($pass, _where(), $name, @mess);
}

sub cmp_ok ($$$@) {
    my($got, $type, $expected, $name, @mess) = @_;

    my $pass;
    {
        local $^W = 0;
        local($@,$!);   # don't interfere with $@
                        # eval() sometimes resets $!
        $pass = eval "\$got $type \$expected";
    }
    unless ($pass) {
        # It seems Irix long doubles can have 2147483648 and 2147483648
        # that stringify to the same thing but are actually numerically
        # different. Display the numbers if $type isn't a string operator,
        # and the numbers are stringwise the same.
        # (all string operators have alphabetic names, so tr/a-z// is true)
        # This will also show numbers for some unneeded cases, but will
        # definitely be helpful for things such as == and <= that fail
        if ($got eq $expected and $type !~ tr/a-z//) {
            unshift @mess, "# $got - $expected = " . ($got - $expected) . "\n";
        }
        unshift(@mess, "#      got "._qq($got)."\n",
                       "# expected $type "._qq($expected)."\n");
    }
    _ok($pass, _where(), $name, @mess);
}

# Check that $got is within $range of $expected
# if $range is 0, then check it's exact
# else if $expected is 0, then $range is an absolute value
# otherwise $range is a fractional error.
# Here $range must be numeric, >= 0
# Non numeric ranges might be a useful future extension. (eg %)
sub within ($$$@) {
    my ($got, $expected, $range, $name, @mess) = @_;
    my $pass;
    if (!defined $got or !defined $expected or !defined $range) {
        # This is a fail, but doesn't need extra diagnostics
    } elsif ($got !~ tr/0-9// or $expected !~ tr/0-9// or $range !~ tr/0-9//) {
        # This is a fail
        unshift @mess, "# got, expected and range must be numeric\n";
    } elsif ($range < 0) {
        # This is also a fail
        unshift @mess, "# range must not be negative\n";
    } elsif ($range == 0) {
        # Within 0 is ==
        $pass = $got == $expected;
    } elsif ($expected == 0) {
        # If expected is 0, treat range as absolute
        $pass = ($got <= $range) && ($got >= - $range);
    } else {
        my $diff = $got - $expected;
        $pass = abs ($diff / $expected) < $range;
    }
    unless ($pass) {
        if ($got eq $expected) {
            unshift @mess, "# $got - $expected = " . ($got - $expected) . "\n";
        }
	unshift@mess, "#      got "._qq($got)."\n",
		      "# expected "._qq($expected)." (within "._qq($range).")\n";
    }
    _ok($pass, _where(), $name, @mess);
}

sub pass {
    _ok(1, '', @_);
}

sub fail {
    _ok(0, _where(), @_);
}

sub curr_test {
    $test = shift if @_;
    return $test;
}

sub next_test {
  my $retval = $test;
  $test = $test + 1; # don't use ++
  $retval;
}

# Note: can't pass multipart messages since we try to
# be compatible with Test::More::skip().
sub skip {
    my $why = shift;
    my $n   = @_ ? shift : 1;
    my $bad_swap;
    my $both_zero;
    {
      local $^W = 0;
      $bad_swap = $why > 0 && $n == 0;
      $both_zero = $why == 0 && $n == 0;
    }
    if ($bad_swap || $both_zero || @_) {
      my $arg = "'$why', '$n'";
      if (@_) {
        $arg .= join(", ", '', map { qq['$_'] } @_);
      }
      die qq[$0: expected skip(why, count), got skip($arg)\n];
    }
    for (1..$n) {
        _print "ok $test # skip $why\n";
        $test = $test + 1;
    }
    local $^W = 0;
    #last SKIP;
}

sub eq_array {
    my ($ra, $rb) = @_;
    return 0 unless $#$ra == $#$rb;
    my $i;
    for $i (0..$#$ra) {
	next     if !defined $ra->[$i] && !defined $rb->[$i];
	return 0 if !defined $ra->[$i];
	return 0 if !defined $rb->[$i];
	return 0 unless $ra->[$i] eq $rb->[$i];
    }
    return 1;
}

sub eq_hash {
  my ($orig, $suspect) = @_;
  my $fail;
  while (my ($key, $value) = each %$suspect) {
    # Force a hash recompute if this perl's internals can cache the hash key.
    $key = "" . $key;
    if (exists $orig->{$key}) {
      if (
        defined $orig->{$key} != defined $value
        || (defined $value && $orig->{$key} ne $value)
      ) {
        _print "# key ", _qq($key), " was ", _qq($orig->{$key}),
                     " now ", _qq($value), "\n";
        $fail = 1;
      }
    } else {
      _print "# key ", _qq($key), " is ", _qq($value),
                   ", not in original.\n";
      $fail = 1;
    }
  }
  foreach (keys %$orig) {
    # Force a hash recompute if this perl's internals can cache the hash key.
    $_ = "" . $_;
    next if (exists $suspect->{$_});
    _print "# key ", _qq($_), " was ", _qq($orig->{$_}), " now missing.\n";
    $fail = 1;
  }
  !$fail;
}

1;
