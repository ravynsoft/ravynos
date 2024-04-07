#!./perl

#
# test the conversion operators
#
# Notations:
#
# "N p i N vs N N":  Apply op-N, then op-p, then op-i, then reporter-N
# Compare with application of op-N, then reporter-N
# Right below are descriptions of different ops and reporters.

# We do not use these subroutines any more, sub overhead makes a "switch"
# solution better:

# obviously, 0, 1 and 2, 3 are destructive.  (XXXX 64-bit? 4 destructive too)

# *0 = sub {--$_[0]};		# -
# *1 = sub {++$_[0]};		# +

# # Converters
# *2 = sub { $_[0] = $max_uv & $_[0]}; # U
# *3 = sub { use integer; $_[0] += $zero}; # I
# *4 = sub { $_[0] += $zero};	# N
# *5 = sub { $_[0] = "$_[0]" };	# P

# # Side effects
# *6 = sub { $max_uv & $_[0]};	# u
# *7 = sub { use integer; $_[0] + $zero};	# i
# *8 = sub { $_[0] + $zero};	# n
# *9 = sub { $_[0] . "" };	# p

# # Reporters
# sub a2 { sprintf "%u", $_[0] }	# U
# sub a3 { sprintf "%d", $_[0] }	# I
# sub a4 { sprintf "%g", $_[0] }	# N
# sub a5 { "$_[0]" }		# P

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    if (pack("d", 1) =~ /^[\x80\10]\x40/) {
        skip_all("VAX float cannot do infinity");
    }
    set_up_inc('../lib');
}

use strict;

my $max_chain = $ENV{PERL_TEST_NUMCONVERTS} || 2;

# Bulk out if unsigned type is hopelessly wrong:
my $max_uv1 = ~0;
my $max_uv2 = sprintf "%u", $max_uv1 ** 6; # 6 is an arbitrary number here
my $big_iv = do {use integer; $max_uv1 * 16}; # 16 is an arbitrary number here
my $max_uv_less3 = $max_uv1 - 3;

print "# max_uv1 = $max_uv1, max_uv2 = $max_uv2, big_iv = $big_iv\n";
print "# max_uv_less3 = $max_uv_less3\n";
if ($max_uv1 ne $max_uv2 or $big_iv > $max_uv1 or $max_uv1 == $max_uv_less3) {
  eval { require Config; };
  my $message = 'unsigned perl arithmetic is not sane';
  $message .= " (common in 64-bit platforms)" if $Config::Config{d_quad};
  skip_all($message);
}
if ($max_uv_less3 =~ tr/0-9//c) {
  skip_all('this perl stringifies large unsigned integers using E notation');
}

my $st_t = 4*4;			# We try 4 initializers and 4 reporters

my $num = 0;
$num += 10**$_ - 4**$_ for 1.. $max_chain;
$num *= $st_t;
$num += $::additional_tests;
plan(tests => $num);		# In fact 15 times more subsubtests...

my $max_uv = ~0;
my $max_iv = int($max_uv/2);
my $zero = 0;

my $l_uv = length $max_uv;
my $l_iv = length $max_iv;

# Hope: the first digits are good
my $larger_than_uv = substr 97 x 100, 0, $l_uv;
my $smaller_than_iv = substr 12 x 100, 0, $l_iv;
my $yet_smaller_than_iv = substr 97 x 100, 0, ($l_iv - 1);

my @list = (1, $yet_smaller_than_iv, $smaller_than_iv, $max_iv, $max_iv + 1,
	    $max_uv, $max_uv + 1);
unshift @list, (reverse map -$_, @list), 0; # 15 elts
@list = map "$_", @list; # Normalize

note("@list");

# need to special case ++ for max_uv, as ++ "magic" on a string gives
# another string, whereas ++ magic on a string used as a number gives
# a number. Not a problem when NV preserves UV, but if it doesn't then
# stringification of the latter gives something in e notation.

my $max_uv_pp = "$max_uv"; $max_uv_pp++;
my $max_uv_p1 = "$max_uv"; $max_uv_p1+=0; $max_uv_p1++;

# Also need to cope with %g notation for max_uv_p1 that actually gives an
# integer less than max_uv because of correct rounding for the limited
# precision. This bites for 12 byte long doubles and 8 byte UVs

my $temp = $max_uv_p1;
my $max_uv_p1_as_iv;
{use integer; $max_uv_p1_as_iv = 0 + sprintf "%s", $temp}
my $max_uv_p1_as_uv = 0 | sprintf "%s", $temp;

my @opnames = split //, "-+UINPuinp";

# @list = map { 2->($_), 3->($_), 4->($_), 5->($_),  } @list; # Prepare input

my $test = 1;
my $nok;
for my $num_chain (1..$max_chain) {
  my @ops = map [split //], grep /[4-9]/,
    map { sprintf "%0${num_chain}d", $_ }  0 .. 10**$num_chain - 1;

  #@ops = ([]) unless $num_chain;
  #@ops = ([6, 4]);

  for my $op (@ops) {
    for my $first (2..5) {
      for my $last (2..5) {
	$nok = 0;
	my @otherops = grep $_ <= 3, @$op;
	my @curops = ($op,\@otherops);

	for my $num (@list) {
	  my $inpt;
	  my @ans;

	  for my $short (0, 1) {
	    # undef $inpt;	# Forget all we had - some bugs were masked

	    $inpt = $num;	# Try to not contaminate $num...
	    $inpt = "$inpt";
	    if ($first == 2) {
	      $inpt = $max_uv & $inpt; # U 2
	    } elsif ($first == 3) {
	      use integer; $inpt += $zero; # I 3
	    } elsif ($first == 4) {
	      $inpt += $zero;	# N 4
	    } else {
	      $inpt = "$inpt";	# P 5
	    }

	    # Saves 20% of time - not with this logic:
	    #my $tmp = $inpt;
	    #my $tmp1 = $num;
	    #next if $num_chain > 1
	    #  and "$tmp" ne "$tmp1"; # Already the coercion gives problems...

	    for my $curop (@{$curops[$short]}) {
	      if ($curop < 5) {
		if ($curop < 3) {
		  if ($curop == 0) {
		    --$inpt;	# - 0
		  } elsif ($curop == 1) {
		    ++$inpt;	# + 1
		  } else {
		    $inpt = $max_uv & $inpt; # U 2
		  }
		} elsif ($curop == 3) {
		  use integer; $inpt += $zero;
		} else {
		  $inpt += $zero; # N 4
		}
	      } elsif ($curop < 8) {
		if ($curop == 5) {
		  $inpt = "$inpt"; # P 5
		} elsif ($curop == 6) {
		  my $dummy = $max_uv & $inpt; # u 6
		} else {
		  use integer; my $dummy = $inpt + $zero;
		}
	      } elsif ($curop == 8) {
		my $dummy = $inpt + $zero;	# n 8
	      } else {
		my $dummy = $inpt . "";	# p 9
	      }
	    }

	    if ($last == 2) {
	      $inpt = sprintf "%u", $inpt; # U 2
	    } elsif ($last == 3) {
	      $inpt = sprintf "%d", $inpt; # I 3
	    } elsif ($last == 4) {
	      $inpt = sprintf "%g", $inpt; # N 4
	    } else {
	      $inpt = "$inpt";	# P 5
	    }
	    push @ans, $inpt;
	  }
	  if ($ans[0] ne $ans[1]) {
	    my $diag = "'$ans[0]' ne '$ans[1]',\t$num\t=> @opnames[$first,@{$curops[0]},$last] vs @opnames[$first,@{$curops[1]},$last]";
	    my $excuse;
	    # XXX ought to check that "+" was in the list of opnames
	    if ((($ans[0] eq $max_uv_pp) and ($ans[1] eq $max_uv_p1))
		or (($ans[1] eq $max_uv_pp) and ($ans[0] eq $max_uv_p1))) {
	      # string ++ versus numeric ++. Tolerate this little
	      # bit of insanity
	      $excuse = "ok, as string ++ of max_uv is \"$max_uv_pp\", numeric is $max_uv_p1";
	    } elsif ($opnames[$last] eq 'I' and $ans[1] eq "-1"
		     and $ans[0] eq $max_uv_p1_as_iv) {
              # Max UV plus 1 is NV. This NV may stringify in E notation.
              # And the number of decimal digits shown in E notation will depend
              # on the binary digits in the mantissa. And it may be that
              # (say)  18446744073709551616 in E notation is truncated to
              # (say) 1.8446744073709551e+19 (say) which gets converted back
              # as    1.8446744073709551000e+19
              # ie    18446744073709551000
              # which isn't the integer we first had.
              # But each step of conversion is correct. So it's not an error.
              # (Only shows up for 64 bit UVs and NVs with 64 bit mantissas,
              #  and on Crays (64 bit integers, 48 bit mantissas) IIRC)
	      $excuse = "ok, \"$max_uv_p1\" correctly converts to IV \"$max_uv_p1_as_iv\"";
	    } elsif ($opnames[$last] eq 'U' and $ans[1] eq ~0
		     and $ans[0] eq $max_uv_p1_as_uv) {
              # as aboce
	      $excuse = "ok, \"$max_uv_p1\" correctly converts to UV \"$max_uv_p1_as_uv\"";
	    } elsif (grep {defined $_ && /^N$/} @opnames[@{$curops[0]}]
		     and $ans[0] == $ans[1] and $ans[0] <= ~0
                     # First must be in E notation (ie not just digits) and
                     # second must still be an integer.
		     # eg 1.84467440737095516e+19
		     # 1.84467440737095516e+19 for 64 bit mantissa is in the
		     # integer range, so 1.84467440737095516e+19 + 0 is treated
		     # as integer addition. [should it be?]
		     # and 18446744073709551600 + 0 is 18446744073709551600
		     # Which isn't the string you first thought of.
                     # I can't remember why there isn't symmetry in this
                     # exception, ie why only the first ops are tested for 'N'
                     and $ans[0] != /^-?\d+$/ and $ans[1] !~ /^-?\d+$/) {
	      $excuse = "ok, numerically equal - notation changed due to adding zero";
	    } else {
	      $nok++,
	      diag($diag);
	    }
	    if ($excuse) {
	      note($diag);
	      note($excuse);
	    }
	  }
	}
	ok($nok == 0);
      }
    }
  }
}

# Tests that use test.pl start here.
BEGIN { $::additional_tests = 4 }

ok(-0.0 eq "0", 'negative zero stringifies as 0');
ok(!-0.0, "neg zero is boolean false");
my $nz = -0.0;
{ my $dummy = "$nz"; }
ok(!$nz, 'previously stringified -0.0 is boolean false');
$nz = -0.0;
is sprintf("%+.f", - -$nz), sprintf("%+.f", - -$nz),
  "negation does not coerce negative zeroes";
