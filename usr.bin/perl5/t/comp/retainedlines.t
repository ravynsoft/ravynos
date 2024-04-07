#!./perl -w

# Check that lines from eval are correctly retained by the debugger

# Uncomment this for testing, but don't leave it in for "production", as
# we've not yet verified that use works.
# use strict;

print "1..109\n";
my $test = 0;

sub failed {
    my ($got, $expected, $name) = @_;

    print "not ok $test - $name\n";
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

sub is($$$) {
    my ($got, $expect, $name) = @_;
    $test = $test + 1;
    if (defined $expect) {
	if (defined $got && $got eq $expect) {
	    print "ok $test - $name\n";
	    return 1;
	}
	failed($got, "'$expect'", $name);
    } else {
	if (!defined $got) {
	    print "ok $test - $name\n";
	    return 1;
	}
	failed($got, 'undef', $name);
    }
}

$^P = 0xA;

my @before = grep { /eval/ } keys %::;

is ((scalar @before), 0, "No evals");

my %seen;

sub check_retained_lines {
    my ($prog, $name) = @_;
    # Is there a more efficient way to write this?
    my @expect_lines = (undef, map ({"$_\n"} split "\n", $prog), "\n", ';');

    # sort in decreasing number so that $keys[0] is the from the most
    # recent eval. In theory we should only have one, but if something
    # breaks we might have more than one, and keys will return them in a
    # random order, so if we dont do this then failing tests will have
    # inconsistent results from run to run.
    my @keys = map { $_->[0] }
               sort { $b->[1] <=> $a->[1] }
               map { (!$seen{$_} and /eval (\d+)/) ? [ $_, $1 ] : ()  }
               keys %::;

    is ((scalar @keys), 1, "1 new eval");

    my @got_lines = @{$::{$keys[0]}};

    is ((scalar @got_lines),
	(scalar @expect_lines), "Right number of lines for $name");

    for (0..$#expect_lines) {
	is ($got_lines[$_], $expect_lines[$_], "Line $_ is correct");
    }
    # if we are "leaking" evals we only want to fail the current test,
    # so we need to mark them all seen (older code only marked $keys[0]
    # seen and this caused tests to fail that actually worked properly.)
    $seen{$_}++ for @keys;
}

my $name = 'foo';

for my $sep (' ', "\0") {

    my $prog = "sub $name {
    'Perl${sep}Rules'
};
1;
";

    eval $prog or die;
    check_retained_lines($prog, ord $sep);
    $name++;
}

{
  # This contains a syntax error
  my $prog = "sub $name {
    'This is $name'
  }
# 10 errors to triger a croak during compilation.
1 +; 1 +; 1 +; 1 +; 1 +;
1 +; 1 +; 1 +; 1 +; 1 +;
1 +; # and one more for good measure.
";

  eval $prog and die;

  is (eval "$name()", "This is $name", "Subroutine was compiled, despite error")
    or print STDERR "# $@\n";

  check_retained_lines($prog,
		       'eval that defines subroutine but has syntax error');
  $name++;
}

foreach my $flags (0x0, 0x800, 0x1000, 0x1800) {
    local $^P = $^P | $flags;
    # This is easier if we accept that the guts eval will add a trailing \n
    # for us
    my $prog = "1 + 1 + 1\n";
    my $fail = "1 +;\n" x 11; # we need 10 errors to trigger a croak during
                              # compile, we add an extra one just for good
                              # measure.

    is (eval $prog, 3, 'String eval works');
    if ($flags & 0x800) {
	check_retained_lines($prog, sprintf "%#X", $^P);
    } else {
	my @after = grep { /eval/ } keys %::;

	is (scalar @after, 0 + keys %seen,
	    "evals that don't define subroutines are correctly cleaned up");
    }

    is (eval $fail, undef, 'Failed string eval fails');

    if ($flags & 0x1000) {
	check_retained_lines($fail, sprintf "%#X", $^P);
    } else {
	my @after = grep { /eval/ } keys %::;

	is (scalar @after, 0 + keys %seen,
	    "evals that fail are correctly cleaned up");
    }
}

# BEGIN blocks that die
for (0xA, 0) {
  local $^P = $_;

  eval (my $prog = "BEGIN{die}\n");

  if ($_) {
    check_retained_lines($prog, 'eval that defines BEGIN that dies');
  }
  else {
    my @after = grep { /eval/ } keys %::;

    is (scalar @after, 0 + keys %seen,
       "evals with BEGIN{die} are correctly cleaned up");
  }
}

for (0xA, 0) {
  local $^P = $_;

  eval (my $prog = "UNITCHECK{die}\n");
  is (!!$@, 1, "Is \$@ true?");
  is ($@=~/UNITCHECK failed--call queue aborted/, 1,
      "Error is expected value?");

  if ($_) {
    check_retained_lines($prog, 'eval that defines UNITCHECK that dies');
  }
  else {
    my @after = grep { /eval/ } keys %::;

    is (scalar @after, 0 + keys %seen,
       "evals with UNITCHECK{die} are correctly cleaned up");
  }
}


# [perl #79442] A #line "foo" directive in a string eval was not updating
# *{"_<foo"} in threaded perls, and was not putting the right lines into
# the right elements of @{"_<foo"} in non-threaded perls.
{
  local $^P = 0x400|0x100|0x10;
  eval qq{#line 42 "hash-line-eval"\n labadalabada()\n};
  is $::{"_<hash-line-eval"}[42], " labadalabada()\n",
   '#line 42 "foo" in a string eval updates @{"_<foo"}';
  eval qq{#line 42 "figgle"\n#line 85 "doggo"\n labadalabada()\n};
  is $::{"_<doggo"}[85], " labadalabada()\n",
   'subsequent #line 42 "foo" in a string eval updates @{"_<foo"}';
}

# Modifying ${"_<foo"} should not stop lines from being retained.
{
  local $^P = 0x400|0x100|0x10;
  eval <<'end';
#line 42 "copfilesv-modification"
    BEGIN{ ${"_<copfilesv-modification"} = \1 }
#line 52 "copfilesv-modified"
    abcdefg();
end
  is $::{"_<copfilesv-modified"}[52], "    abcdefg();\n",
   '#line 42 "foo" in a str eval is not confused by ${"_<foo"} changing';
}
