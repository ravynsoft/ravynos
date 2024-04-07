#!./perl

# Tests the scoping of $^H and %^H

BEGIN {
    @INC = qw(. ../lib ../ext/re);
    chdir 't' if -d 't';
}

BEGIN { print "1..31\n"; }
BEGIN {
    print "not " if exists $^H{foo};
    print "ok 1 - \$^H{foo} doesn't exist initially\n";
    if (${^OPEN}) {
	print "not " unless $^H & 0x00020000;
	print "ok 2 - \$^H contains HINT_LOCALIZE_HH initially with ${^OPEN}\n";
    } else {
	print "not " if $^H & 0x00020000;
	print "ok 2 - \$^H doesn't contain HINT_LOCALIZE_HH initially\n";
    }
}
{
    # simulate a pragma -- don't forget HINT_LOCALIZE_HH
    BEGIN { $^H |= 0x04020000; $^H{foo} = "a"; }
    BEGIN {
	print "not " if $^H{foo} ne "a";
	print "ok 3 - \$^H{foo} is now 'a'\n";
	print "not " unless $^H & 0x00020000;
	print "ok 4 - \$^H contains HINT_LOCALIZE_HH while compiling\n";
    }
    {
	BEGIN { $^H |= 0x00020000; $^H{foo} = "b"; }
	BEGIN {
	    print "not " if $^H{foo} ne "b";
	    print "ok 5 - \$^H{foo} is now 'b'\n";
	}
    }
    BEGIN {
	print "not " if $^H{foo} ne "a";
	print "ok 6 - \$^H{foo} restored to 'a'\n";
    }
    # The pragma settings disappear after compilation
    # (test at CHECK-time and at run-time)
    CHECK {
	print "not " if exists $^H{foo};
	print "ok 9 - \$^H{foo} doesn't exist when compilation complete\n";
	if (${^OPEN}) {
	    print "not " unless $^H & 0x00020000;
	    print "ok 10 - \$^H contains HINT_LOCALIZE_HH when compilation complete with ${^OPEN}\n";
	} else {
	    print "not " if $^H & 0x00020000;
	    print "ok 10 - \$^H doesn't contain HINT_LOCALIZE_HH when compilation complete\n";
	}
    }
    print "not " if exists $^H{foo};
    print "ok 11 - \$^H{foo} doesn't exist at runtime\n";
    if (${^OPEN}) {
	print "not " unless $^H & 0x00020000;
	print "ok 12 - \$^H contains HINT_LOCALIZE_HH at run-time with ${^OPEN}\n";
    } else {
	print "not " if $^H & 0x00020000;
	print "ok 12 - \$^H doesn't contain HINT_LOCALIZE_HH at run-time\n";
    }
    # op_entereval should keep the pragmas it was compiled with
    eval q*
      BEGIN {
	print "not " if $^H{foo} ne "a";
	print "ok 13 - \$^H{foo} is 'a' at eval-\"\" time\n";
	print "not " unless $^H & 0x00020000;
	print "ok 14 - \$^H contains HINT_LOCALIZE_HH at eval\"\"-time\n";
      }
    *;
}
BEGIN {
    print "not " if exists $^H{foo};
    print "ok 7 - \$^H{foo} doesn't exist while finishing compilation\n";
    if (${^OPEN}) {
	print "not " unless $^H & 0x00020000;
	print "ok 8 - \$^H contains HINT_LOCALIZE_HH while finishing compilation with ${^OPEN}\n";
    } else {
	print "not " if $^H & 0x00020000;
	print "ok 8 - \$^H doesn't contain HINT_LOCALIZE_HH while finishing compilation\n";
    }
}

{
    BEGIN{$^H{x}=1};
    for my $tno (15..16) {
        eval q(
            BEGIN {
                print $^H{x}==1 && !$^H{y} ? "ok $tno\n" : "not ok $tno\n";
            }
            $^H{y} = 1;
        );
        if ($@) {
            (my $str = $@)=~s/^/# /gm;
            print "not ok $tno\n$str\n";
        }
    }
}

{
    BEGIN { $^H |= 0x04000000; $^H{foo} = "z"; }

    our($ri0, $rf0); BEGIN { $ri0 = $^H; $rf0 = $^H{foo}; }
    print +($ri0 & 0x04000000 ? "" : "not "), "ok 17 - \$^H correct before require\n";
    print +($rf0 eq "z" ? "" : "not "), "ok 18 - \$^H{foo} correct before require\n";

    our($ra1, $ri1, $rf1, $rfe1);
    BEGIN { require "comp/hints.aux"; }
    print +(!($ri1 & 0x04000000) ? "" : "not "), "ok 19 - \$^H cleared for require\n";
    print +(!defined($rf1) && !$rfe1 ? "" : "not "), "ok 20 - \$^H{foo} cleared for require\n";

    our($ri2, $rf2); BEGIN { $ri2 = $^H; $rf2 = $^H{foo}; }
    print +($ri2 & 0x04000000 ? "" : "not "), "ok 21 - \$^H correct after require\n";
    print +($rf2 eq "z" ? "" : "not "), "ok 22 - \$^H{foo} correct after require\n";
}

# [perl #73174]

{
    my $res;
    BEGIN { $^H{73174} = "foo" }
    BEGIN { $res = ($^H{73174} // "") }
    # /x{100}/i forces loading of utf8.pm, which used to reset %^H
    eval '"" =~ /\x{100}/i; 1'
	# Allow miniperl to fail this regexp compilation (effectively skip
	# the test) in case tables have not been build, but require real
	# perl to succeed.
	or defined &DynaLoader::boot_DynaLoader and die;	
    BEGIN { $res .= '-' . ($^H{73174} // "")}
    $res .= '-' . ($^H{73174} // "");
    print $res eq "foo-foo-" ? "" : "not ",
	"ok 23 - \$^H{foo} correct after /unicode/i (res=$res)\n";
}

# [perl #106282] Crash when tying %^H
# Tying %^H should not result in a crash when the hint hash is cloned.
# Hints should also be copied properly to inner scopes.  See also
# [rt.cpan.org #73402].
eval q`
    # Do something naughty enough, and you get your module mentioned in the
    # test suite. :-)
    package namespace::clean::_TieHintHash;

    sub TIEHASH  { bless[] }
    sub STORE    { $_[0][0]{$_[1]} = $_[2] }
    sub FETCH    { $_[0][0]{$_[1]} }
    sub FIRSTKEY { my $a = scalar keys %{$_[0][0]}; each %{$_[0][0]} }
    sub NEXTKEY  { each %{$_[0][0]} }

    package main;

    BEGIN {
	$^H{foo} = "bar"; # activate localisation magic
	tie( %^H, 'namespace::clean::_TieHintHash' ); # sabotage %^H
	$^H{foo} = "bar"; # create an element in the tied hash
    }
    { # clone the tied hint hash on scope entry
	BEGIN {
	    print "not " x ($^H{foo} ne 'bar'),
		  "ok 24 - tied hint hash is copied to inner scope\n";
	    %^H = ();
	    tie( %^H, 'namespace::clean::_TieHintHash' );
	    $^H{foo} = "bar";
	}
	{
	    BEGIN{
		print
		  "not " x ($^H{foo} ne 'bar'),
		  "ok 25 - tied empty hint hash is copied to inner scope\n"
	    }    
	}
	1;
    }
    1;
` or warn $@;
print "ok 26 - no crash when cloning a tied hint hash\n";

{
    my $w;
    local $SIG{__WARN__} = sub { $w = shift };
    eval q`
	package namespace::clean::_TieHintHasi;
    
	sub TIEHASH  { bless[] }
	sub STORE    { $_[0][0]{$_[1]} = $_[2] }
	sub FETCH    { $_[0][0]{$_[1]} }
	sub FIRSTKEY { my $a = scalar keys %{$_[0][0]}; each %{$_[0][0]} }
      # Intentionally commented out:
      #  sub NEXTKEY  { each %{$_[0][0]} }
    
	package main;
    
	BEGIN {
    	    $^H{foo} = "bar"; # activate localisation magic
    	    tie( %^H, 'namespace::clean::_TieHintHasi' ); # sabotage %^H
    	    $^H{foo} = "bar"; # create an element in the tied hash
	}
	{ ; } # clone the tied hint hash
    `;
    print "not " if $w;
    print "ok 27 - double-freeing explosive tied hints hash\n";
    print "# got: $w" if $w;
}

# Setting ${^WARNING_HINTS} to its own value should not change things.
{
    my $w;
    local $SIG{__WARN__} = sub { $w++ };
    BEGIN {
	# should have no effect:
	my $x = ${^WARNING_BITS};
	${^WARNING_BITS} = $x;
    }
    {
	local $^W = 1;
	() = 1 + undef;
    }
    print "# ", $w//'no', " warnings\nnot " unless $w == 1;
    print "ok 28 - ",
          "setting \${^WARNING_BITS} to its own value has no effect\n";
}

# [perl #112326]
# this code could cause a crash, due to PL_hints continuing to point to th
# hints hash currently being freed

{
    package Foo;
    my @h = qw(a 1 b 2);
    BEGIN {
	$^H{FOO} = bless {};
    }
    sub DESTROY {
	@h = %^H;
	delete $INC{strict}; require strict; # boom!
    }
    my $h = join ':', %h;
    # this isn't the main point of the test; the main point is that
    # it doesn't crash!
    print "not " if $h ne '';
    print "ok 29 - #112326\n";
}


# [perl #112444]
# A destructor called while %^H is freed should not be able to stop %^H
# from being magical (due to *^H{HASH} being undef).
{
    BEGIN {
	# Make sure %^H is clear and not localised, to begin with
	%^H = ();
	$^H = 0;
    }
    DESTROY { %^H }
    {
	{
	    BEGIN {
		$^H{foom} = bless[];
	    }
	} # scope exit triggers destructor, which autovivifies a non-
	  # magical %^H
	BEGIN {
	    # Here we have the %^H created by DESTROY, which is
	    # not localised
	    $^H{112444} = 'baz';
	}
    } # %^H leaks on scope exit
    BEGIN { @keez = keys %^H }
}
print "not " if @keez;
print "ok 30 - %^H does not leak when autovivified in destructor\n";
print "# keys are: @keez\n" if @keez;


# Add new tests above this require, in case it fails.
require './test.pl';

# bug #27040: hints hash was being double-freed
my $result = runperl(
    prog => '$^H |= 0x20000; eval q{BEGIN { $^H |= 0x20000 }}',
    stderr => 1
);
print "not " if length $result;
print "ok 31 - double-freeing hints hash\n";
print "# got: $result\n" if length $result;

__END__
# Add new tests above require 'test.pl'
