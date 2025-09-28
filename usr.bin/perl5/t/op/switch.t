#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use warnings;
no warnings 'deprecated';

plan tests => 197;

# The behaviour of the feature pragma should be tested by lib/feature.t
# using the tests in t/lib/feature/*. This file tests the behaviour of
# the switch ops themselves.


# Before loading feature, test the switch ops with CORE::
CORE::given(3) {
    CORE::when(3) { pass "CORE::given and CORE::when"; continue }
    CORE::default { pass "continue (without feature) and CORE::default" }
}


use feature 'switch';

eval { continue };
like($@, qr/^Can't "continue" outside/, "continue outside");

eval { break };
like($@, qr/^Can't "break" outside/, "break outside");

# Scoping rules

{
    my $x = "foo";
    given(my $x = "bar") {
	is($x, "bar", "given scope starts");
    }
    is($x, "foo", "given scope ends");
}

sub be_true {1}

given(my $x = "foo") {
    when(be_true(my $x = "bar")) {
	is($x, "bar", "given scope starts");
    }
    is($x, "foo", "given scope ends");
}

$_ = "outside";
given("inside") { check_outside1() }
sub check_outside1 { is($_, "inside", "\$_ is not lexically scoped") }

# Basic string/numeric comparisons and control flow

{    
    my $ok;
    given(3) {
	when(2) { $ok = 'two'; }
	when(3) { $ok = 'three'; }
	when(4) { $ok = 'four'; }
	default { $ok = 'd'; }
    }
    is($ok, 'three', "numeric comparison");
}

{    
    my $ok;
    use integer;
    given(3.14159265) {
	when(2) { $ok = 'two'; }
	when(3) { $ok = 'three'; }
	when(4) { $ok = 'four'; }
	default { $ok = 'd'; }
    }
    is($ok, 'three', "integer comparison");
}

{    
    my ($ok1, $ok2);
    given(3) {
	when(3.1)   { $ok1 = 'n'; }
	when(3.0)   { $ok1 = 'y'; continue }
	when("3.0") { $ok2 = 'y'; }
	default     { $ok2 = 'n'; }
    }
    is($ok1, 'y', "more numeric (pt. 1)");
    is($ok2, 'y', "more numeric (pt. 2)");
}

{
    my $ok;
    given("c") {
	when("b") { $ok = 'B'; }
	when("c") { $ok = 'C'; }
	when("d") { $ok = 'D'; }
	default   { $ok = 'def'; }
    }
    is($ok, 'C', "string comparison");
}

{
    my $ok;
    given("c") {
	when("b") { $ok = 'B'; }
	when("c") { $ok = 'C'; continue }
	when("c") { $ok = 'CC'; }
	default   { $ok = 'D'; }
    }
    is($ok, 'CC', "simple continue");
}

# Definedness
{
    my $ok = 1;
    given (0) { when(undef) {$ok = 0} }
    is($ok, 1, "Given(0) when(undef)");
}
{
    my $undef;
    my $ok = 1;
    given (0) { when($undef) {$ok = 0} }
    is($ok, 1, 'Given(0) when($undef)');
}
{
    my $undef;
    my $ok = 0;
    given (0) { when($undef++) {$ok = 1} }
    is($ok, 1, "Given(0) when($undef++)");
}
{
    no warnings "uninitialized";
    my $ok = 1;
    given (undef) { when(0) {$ok = 0} }
    is($ok, 1, "Given(undef) when(0)");
}
{
    no warnings "uninitialized";
    my $undef;
    my $ok = 1;
    given ($undef) { when(0) {$ok = 0} }
    is($ok, 1, 'Given($undef) when(0)');
}
########
{
    my $ok = 1;
    given ("") { when(undef) {$ok = 0} }
    is($ok, 1, 'Given("") when(undef)');
}
{
    my $undef;
    my $ok = 1;
    given ("") { when($undef) {$ok = 0} }
    is($ok, 1, 'Given("") when($undef)');
}
{
    no warnings "uninitialized";
    my $ok = 1;
    given (undef) { when("") {$ok = 0} }
    is($ok, 1, 'Given(undef) when("")');
}
{
    no warnings "uninitialized";
    my $undef;
    my $ok = 1;
    given ($undef) { when("") {$ok = 0} }
    is($ok, 1, 'Given($undef) when("")');
}
########
{
    my $ok = 0;
    given (undef) { when(undef) {$ok = 1} }
    is($ok, 1, "Given(undef) when(undef)");
}
{
    my $undef;
    my $ok = 0;
    given (undef) { when($undef) {$ok = 1} }
    is($ok, 1, 'Given(undef) when($undef)');
}
{
    my $undef;
    my $ok = 0;
    given ($undef) { when(undef) {$ok = 1} }
    is($ok, 1, 'Given($undef) when(undef)');
}
{
    my $undef;
    my $ok = 0;
    given ($undef) { when($undef) {$ok = 1} }
    is($ok, 1, 'Given($undef) when($undef)');
}


# Regular expressions
{
    my ($ok1, $ok2);
    given("Hello, world!") {
	when(/lo/)
	    { $ok1 = 'y'; continue}
	when(/no/)
	    { $ok1 = 'n'; continue}
	when(/^(Hello,|Goodbye cruel) world[!.?]/)
	    { $ok2 = 'Y'; continue}
	when(/^(Hello cruel|Goodbye,) world[!.?]/)
	    { $ok2 = 'n'; continue}
    }
    is($ok1, 'y', "regex 1");
    is($ok2, 'Y', "regex 2");
}

# Comparisons
{
    my $test = "explicit numeric comparison (<)";
    my $twenty_five = 25;
    my $ok;
    given($twenty_five) {
	when ($_ < 10) { $ok = "ten" }
	when ($_ < 20) { $ok = "twenty" }
	when ($_ < 30) { $ok = "thirty" }
	when ($_ < 40) { $ok = "forty" }
	default        { $ok = "default" }
    }
    is($ok, "thirty", $test);
}

{
    use integer;
    my $test = "explicit numeric comparison (integer <)";
    my $twenty_five = 25;
    my $ok;
    given($twenty_five) {
	when ($_ < 10) { $ok = "ten" }
	when ($_ < 20) { $ok = "twenty" }
	when ($_ < 30) { $ok = "thirty" }
	when ($_ < 40) { $ok = "forty" }
	default        { $ok = "default" }
    }
    is($ok, "thirty", $test);
}

{
    my $test = "explicit numeric comparison (<=)";
    my $twenty_five = 25;
    my $ok;
    given($twenty_five) {
	when ($_ <= 10) { $ok = "ten" }
	when ($_ <= 20) { $ok = "twenty" }
	when ($_ <= 30) { $ok = "thirty" }
	when ($_ <= 40) { $ok = "forty" }
	default         { $ok = "default" }
    }
    is($ok, "thirty", $test);
}

{
    use integer;
    my $test = "explicit numeric comparison (integer <=)";
    my $twenty_five = 25;
    my $ok;
    given($twenty_five) {
	when ($_ <= 10) { $ok = "ten" }
	when ($_ <= 20) { $ok = "twenty" }
	when ($_ <= 30) { $ok = "thirty" }
	when ($_ <= 40) { $ok = "forty" }
	default         { $ok = "default" }
    }
    is($ok, "thirty", $test);
}


{
    my $test = "explicit numeric comparison (>)";
    my $twenty_five = 25;
    my $ok;
    given($twenty_five) {
	when ($_ > 40) { $ok = "forty" }
	when ($_ > 30) { $ok = "thirty" }
	when ($_ > 20) { $ok = "twenty" }
	when ($_ > 10) { $ok = "ten" }
	default        { $ok = "default" }
    }
    is($ok, "twenty", $test);
}

{
    my $test = "explicit numeric comparison (>=)";
    my $twenty_five = 25;
    my $ok;
    given($twenty_five) {
	when ($_ >= 40) { $ok = "forty" }
	when ($_ >= 30) { $ok = "thirty" }
	when ($_ >= 20) { $ok = "twenty" }
	when ($_ >= 10) { $ok = "ten" }
	default         { $ok = "default" }
    }
    is($ok, "twenty", $test);
}

{
    use integer;
    my $test = "explicit numeric comparison (integer >)";
    my $twenty_five = 25;
    my $ok;
    given($twenty_five) {
	when ($_ > 40) { $ok = "forty" }
	when ($_ > 30) { $ok = "thirty" }
	when ($_ > 20) { $ok = "twenty" }
	when ($_ > 10) { $ok = "ten" }
	default        { $ok = "default" }
    }
    is($ok, "twenty", $test);
}

{
    use integer;
    my $test = "explicit numeric comparison (integer >=)";
    my $twenty_five = 25;
    my $ok;
    given($twenty_five) {
	when ($_ >= 40) { $ok = "forty" }
	when ($_ >= 30) { $ok = "thirty" }
	when ($_ >= 20) { $ok = "twenty" }
	when ($_ >= 10) { $ok = "ten" }
	default         { $ok = "default" }
    }
    is($ok, "twenty", $test);
}


{
    my $test = "explicit string comparison (lt)";
    my $twenty_five = "25";
    my $ok;
    given($twenty_five) {
	when ($_ lt "10") { $ok = "ten" }
	when ($_ lt "20") { $ok = "twenty" }
	when ($_ lt "30") { $ok = "thirty" }
	when ($_ lt "40") { $ok = "forty" }
	default           { $ok = "default" }
    }
    is($ok, "thirty", $test);
}

{
    my $test = "explicit string comparison (le)";
    my $twenty_five = "25";
    my $ok;
    given($twenty_five) {
	when ($_ le "10") { $ok = "ten" }
	when ($_ le "20") { $ok = "twenty" }
	when ($_ le "30") { $ok = "thirty" }
	when ($_ le "40") { $ok = "forty" }
	default           { $ok = "default" }
    }
    is($ok, "thirty", $test);
}

{
    my $test = "explicit string comparison (gt)";
    my $twenty_five = 25;
    my $ok;
    given($twenty_five) {
	when ($_ ge "40") { $ok = "forty" }
	when ($_ ge "30") { $ok = "thirty" }
	when ($_ ge "20") { $ok = "twenty" }
	when ($_ ge "10") { $ok = "ten" }
	default           { $ok = "default" }
    }
    is($ok, "twenty", $test);
}

{
    my $test = "explicit string comparison (ge)";
    my $twenty_five = 25;
    my $ok;
    given($twenty_five) {
	when ($_ ge "40") { $ok = "forty" }
	when ($_ ge "30") { $ok = "thirty" }
	when ($_ ge "20") { $ok = "twenty" }
	when ($_ ge "10") { $ok = "ten" }
	default           { $ok = "default" }
    }
    is($ok, "twenty", $test);
}

# Optimized-away comparisons
{
    my $ok;
    given(23) {
	when (2 + 2 == 4) { $ok = 'y'; continue }
	when (2 + 2 == 5) { $ok = 'n' }
    }
    is($ok, 'y', "Optimized-away comparison");
}

{
    my $ok;
    given(23) {
        when (scalar 24) { $ok = 'n'; continue }
        default { $ok = 'y' }
    }
    is($ok,'y','scalar()');
}

# File tests
#  (How to be both thorough and portable? Pinch a few ideas
#  from t/op/filetest.t. We err on the side of portability for
#  the time being.)

{
    my ($ok_d, $ok_f, $ok_r);
    given("op") {
	when(-d)  {$ok_d = 1; continue}
	when(!-f) {$ok_f = 1; continue}
	when(-r)  {$ok_r = 1; continue}
    }
    ok($ok_d, "Filetest -d");
    ok($ok_f, "Filetest -f");
    ok($ok_r, "Filetest -r");
}

# Sub and method calls
sub notfoo {"bar"}
{
    my $ok = 0;
    given("foo") {
	when(notfoo()) {$ok = 1}
    }
    ok($ok, "Sub call acts as boolean")
}

{
    my $ok = 0;
    given("foo") {
	when(main->notfoo()) {$ok = 1}
    }
    ok($ok, "Class-method call acts as boolean")
}

{
    my $ok = 0;
    my $obj = bless [];
    given("foo") {
	when($obj->notfoo()) {$ok = 1}
    }
    ok($ok, "Object-method call acts as boolean")
}

# Other things that should not be smart matched
{
    my $ok = 0;
    given(12) {
        when( /(\d+)/ and ( 1 <= $1 and $1 <= 12 ) ) {
            $ok = 1;
        }
    }
    ok($ok, "bool not smartmatches");
}

{
    my $ok = 0;
    given(0) {
	when(eof(DATA)) {
	    $ok = 1;
	}
    }
    ok($ok, "eof() not smartmatched");
}

{
    my $ok = 0;
    my %foo = ("bar", 0);
    given(0) {
	when(exists $foo{bar}) {
	    $ok = 1;
	}
    }
    ok($ok, "exists() not smartmatched");
}

{
    my $ok = 0;
    given(0) {
	when(defined $ok) {
	    $ok = 1;
	}
    }
    ok($ok, "defined() not smartmatched");
}

{
    my $ok = 1;
    given("foo") {
	when((1 == 1) && "bar") {
	    $ok = 0;
	}
	when((1 == 1) && $_ eq "foo") {
	    $ok = 2;
	}
    }
    is($ok, 2, "((1 == 1) && \"bar\") not smartmatched");
}

{
    my $n = 0;
    for my $l (qw(a b c d)) {
	given ($l) {
	    when ($_ eq "b" .. $_ eq "c") { $n = 1 }
	    default { $n = 0 }
	}
	ok(($n xor $l =~ /[ad]/), 'when(E1..E2) evaluates in boolean context');
    }
}

{
    my $n = 0;
    for my $l (qw(a b c d)) {
	given ($l) {
	    when ($_ eq "b" ... $_ eq "c") { $n = 1 }
	    default { $n = 0 }
	}
	ok(($n xor $l =~ /[ad]/), 'when(E1...E2) evaluates in boolean context');
    }
}

{
    my $ok = 0;
    given("foo") {
	when((1 == $ok) || "foo") {
	    $ok = 1;
	}
    }
    ok($ok, '((1 == $ok) || "foo") smartmatched');
}

{
    my $ok = 0;
    given("foo") {
	when((1 == $ok || undef) // "foo") {
	    $ok = 1;
	}
    }
    ok($ok, '((1 == $ok || undef) // "foo") smartmatched');
}

# Make sure we aren't invoking the get-magic more than once

{ # A helper class to count the number of accesses.
    package FetchCounter;
    sub TIESCALAR {
	my ($class) = @_;
	bless {value => undef, count => 0}, $class;
    }
    sub STORE {
        my ($self, $val) = @_;
        $self->{count} = 0;
        $self->{value} = $val;
    }
    sub FETCH {
	my ($self) = @_;
	# Avoid pre/post increment here
	$self->{count} = 1 + $self->{count};
	$self->{value};
    }
    sub count {
	my ($self) = @_;
	$self->{count};
    }
}

my $f = tie my $v, "FetchCounter";

{   my $test_name = "Multiple FETCHes in given, due to aliasing";
    my $ok;
    given($v = 23) {
    	when(undef) {}
    	when(sub{0}->()) {}
	when(21) {}
	when("22") {}
	when(23) {$ok = 1}
	when(/24/) {$ok = 0}
    }
    is($ok, 1, "precheck: $test_name");
    is($f->count(), 4, $test_name);
}

{   my $test_name = "Only one FETCH (numeric when)";
    my $ok;
    $v = 23;
    is($f->count(), 0, "Sanity check: $test_name");
    given(23) {
    	when(undef) {}
    	when(sub{0}->()) {}
	when(21) {}
	when("22") {}
	when($v) {$ok = 1}
	when(/24/) {$ok = 0}
    }
    is($ok, 1, "precheck: $test_name");
    is($f->count(), 1, $test_name);
}

{   my $test_name = "Only one FETCH (string when)";
    my $ok;
    $v = "23";
    is($f->count(), 0, "Sanity check: $test_name");
    given("23") {
    	when(undef) {}
    	when(sub{0}->()) {}
	when("21") {}
	when("22") {}
	when($v) {$ok = 1}
	when(/24/) {$ok = 0}
    }
    is($ok, 1, "precheck: $test_name");
    is($f->count(), 1, $test_name);
}

{   my $test_name = "Only one FETCH (undef)";
    my $ok;
    $v = undef;
    is($f->count(), 0, "Sanity check: $test_name");
    no warnings "uninitialized";
    given(my $undef) {
    	when(sub{0}->()) {}
	when("21")  {}
	when("22")  {}
    	when($v)    {$ok = 1}
	when(undef) {$ok = 0}
    }
    is($ok, 1, "precheck: $test_name");
    is($f->count(), 1, $test_name);
}

# Loop topicalizer
{
    my $first = 1;
    for (1, "two") {
	when ("two") {
	    is($first, 0, "Loop: second");
	    eval {break};
	    like($@, qr/^Can't "break" in a loop topicalizer/,
	    	q{Can't "break" in a loop topicalizer});
	}
	when (1) {
	    is($first, 1, "Loop: first");
	    $first = 0;
	    # Implicit break is okay
	}
    }
}

{
    my $first = 1;
    for $_ (1, "two") {
	when ("two") {
	    is($first, 0, "Explicit \$_: second");
	    eval {break};
	    like($@, qr/^Can't "break" in a loop topicalizer/,
	    	q{Can't "break" in a loop topicalizer});
	}
	when (1) {
	    is($first, 1, "Explicit \$_: first");
	    $first = 0;
	    # Implicit break is okay
	}
    }
}


# Code references
{
    my $called_foo = 0;
    sub foo {$called_foo = 1; "@_" eq "foo"}
    my $called_bar = 0;
    sub bar {$called_bar = 1; "@_" eq "bar"}
    my ($matched_foo, $matched_bar) = (0, 0);
    given("foo") {
	when(\&bar) {$matched_bar = 1}
	when(\&foo) {$matched_foo = 1}
    }
    is($called_foo, 1,  "foo() was called");
    is($called_bar, 1,  "bar() was called");
    is($matched_bar, 0, "bar didn't match");
    is($matched_foo, 1, "foo did match");
}

sub contains_x {
    my $x = shift;
    return ($x =~ /x/);
}
{
    my ($ok1, $ok2) = (0,0);
    given("foxy!") {
	when(contains_x($_))
	    { $ok1 = 1; continue }
	when(\&contains_x)
	    { $ok2 = 1; continue }
    }
    is($ok1, 1, "Calling sub directly (true)");
    is($ok2, 1, "Calling sub indirectly (true)");

    given("foggy") {
	when(contains_x($_))
	    { $ok1 = 2; continue }
	when(\&contains_x)
	    { $ok2 = 2; continue }
    }
    is($ok1, 1, "Calling sub directly (false)");
    is($ok2, 1, "Calling sub indirectly (false)");
}

{
    # Test overloading
    { package OverloadTest;

      use overload '""' => sub{"string value of obj"};
      use overload 'eq' => sub{"$_[0]" eq "$_[1]"};

      use overload "~~" => sub {
	  my ($self, $other, $reversed) = @_;
	  if ($reversed) {
	      $self->{left}  = $other;
	      $self->{right} = $self;
	      $self->{reversed} = 1;
	  } else {
	      $self->{left}  = $self;
	      $self->{right} = $other;
	      $self->{reversed} = 0;
	  }
	  $self->{called} = 1;
	  return $self->{retval};
      };
    
      sub new {
	  my ($pkg, $retval) = @_;
	  bless {
		 called => 0,
		 retval => $retval,
		}, $pkg;
      }
    }

    {
	my $test = "Overloaded obj in given (true)";
	my $obj = OverloadTest->new(1);
	my $matched;
	given($obj) {
	    when ("other arg") {$matched = 1}
	    default {$matched = 0}
	}
    
	is($obj->{called}, 1, "$test: called");
	ok($matched, "$test: matched");
    }

    {
	my $test = "Overloaded obj in given (false)";
	my $obj = OverloadTest->new(0);
	my $matched;
	given($obj) {
	    when ("other arg") {$matched = 1}
	}
    
	is($obj->{called}, 1, "$test: called");
	ok(!$matched, "$test: not matched");
    }

    {
	my $test = "Overloaded obj in when (true)";
	my $obj = OverloadTest->new(1);
	my $matched;
	given("topic") {
	    when ($obj) {$matched = 1}
	    default {$matched = 0}
	}
    
	is($obj->{called},  1, "$test: called");
	ok($matched, "$test: matched");
	is($obj->{left}, "topic", "$test: left");
	is($obj->{right}, "string value of obj", "$test: right");
	ok($obj->{reversed}, "$test: reversed");
    }

    {
	my $test = "Overloaded obj in when (false)";
	my $obj = OverloadTest->new(0);
	my $matched;
	given("topic") {
	    when ($obj) {$matched = 1}
	    default {$matched = 0}
	}
    
	is($obj->{called}, 1, "$test: called");
	ok(!$matched, "$test: not matched");
	is($obj->{left}, "topic", "$test: left");
	is($obj->{right}, "string value of obj", "$test: right");
	ok($obj->{reversed}, "$test: reversed");
    }
}

# Postfix when
{
    my $ok;
    given (undef) {
	$ok = 1 when undef;
    }
    is($ok, 1, "postfix undef");
}
{
    my $ok;
    given (2) {
	$ok += 1 when 7;
	$ok += 2 when 9.1685;
	$ok += 4 when $_ > 4;
	$ok += 8 when $_ < 2.5;
    }
    is($ok, 8, "postfix numeric");
}
{
    my $ok;
    given ("apple") {
	$ok = 1, continue when $_ eq "apple";
	$ok += 2;
	$ok = 0 when "banana";
    }
    is($ok, 3, "postfix string");
}
{
    my $ok;
    given ("pear") {
	do { $ok = 1; continue } when /pea/;
	$ok += 2;
	$ok = 0 when /pie/;
	default { $ok += 4 }
	$ok = 0;
    }
    is($ok, 7, "postfix regex");
}
# be_true is defined at the beginning of the file
{
    my $x = "what";
    given(my $x = "foo") {
	do {
	    is($x, "foo", "scope inside ... when my \$x = ...");
	    continue;
	} when be_true(my $x = "bar");
	is($x, "bar", "scope after ... when my \$x = ...");
    }
}
{
    my $x = 0;
    given(my $x = 1) {
	my $x = 2, continue when be_true();
        is($x, undef, "scope after my \$x = ... when ...");
    }
}

# Tests for last and next in when clauses
my $letter;

$letter = '';
for ("a".."e") {
    given ($_) {
	$letter = $_;
	when ("b") { last }
    }
    $letter = "z";
}
is($letter, "b", "last in when");

$letter = '';
LETTER1: for ("a".."e") {
    given ($_) {
	$letter = $_;
	when ("b") { last LETTER1 }
    }
    $letter = "z";
}
is($letter, "b", "last LABEL in when");

$letter = '';
for ("a".."e") {
    given ($_) {
	when (/b|d/) { next }
	$letter .= $_;
    }
    $letter .= ',';
}
is($letter, "a,c,e,", "next in when");

$letter = '';
LETTER2: for ("a".."e") {
    given ($_) {
	when (/b|d/) { next LETTER2 }
	$letter .= $_;
    }
    $letter .= ',';
}
is($letter, "a,c,e,", "next LABEL in when");

# Test goto with given/when
{
    my $flag = 0;
    goto GIVEN1;
    $flag = 1;
    GIVEN1: given ($flag) {
	when (0) { break; }
	$flag = 2;
    }
    is($flag, 0, "goto GIVEN1");
}
{
    my $flag = 0;
    given ($flag) {
	when (0) { $flag = 1; }
	goto GIVEN2;
	$flag = 2;
    }
GIVEN2:
    is($flag, 1, "goto inside given");
}
{
    my $flag = 0;
    given ($flag) {
	when (0) { $flag = 1; goto GIVEN3; $flag = 2; }
	$flag = 3;
    }
GIVEN3:
    is($flag, 1, "goto inside given and when");
}
{
    my $flag = 0;
    for ($flag) {
	when (0) { $flag = 1; goto GIVEN4; $flag = 2; }
	$flag = 3;
    }
GIVEN4:
    is($flag, 1, "goto inside for and when");
}
{
    my $flag = 0;
GIVEN5:
    given ($flag) {
	when (0) { $flag = 1; goto GIVEN5; $flag = 2; }
	when (1) { break; }
	$flag = 3;
    }
    is($flag, 1, "goto inside given and when to the given stmt");
}

# test with unreified @_ in smart match [perl #71078]
sub unreified_check { ok([@_] ~~ \@_) } # should always match
unreified_check(1,2,"lala");
unreified_check(1,2,undef);
unreified_check(undef);
unreified_check(undef,"");

# Test do { given } as a rvalue

{
    # Simple scalar
    my $lexical = 5;
    my @things = (11 .. 26); # 16 elements
    my @exp = (5, 16, 9);
    no warnings 'void';
    for (0, 1, 2) {
	my $scalar = do { given ($_) {
	    when (0) { $lexical }
	    when (2) { 'void'; 8, 9 }
	    @things;
	} };
	is($scalar, shift(@exp), "rvalue given - simple scalar [$_]");
    }
}
{
    # Postfix scalar
    my $lexical = 5;
    my @exp = (5, 7, 9);
    for (0, 1, 2) {
	no warnings 'void';
	my $scalar = do { given ($_) {
	    $lexical when 0;
	    8, 9     when 2;
	    6, 7;
	} };
	is($scalar, shift(@exp), "rvalue given - postfix scalar [$_]");
    }
}
{
    # Default scalar
    my @exp = (5, 9, 9);
    for (0, 1, 2) {
	my $scalar = do { given ($_) {
	    no warnings 'void';
	    when (0) { 5 }
	    default  { 8, 9 }
	    6, 7;
	} };
	is($scalar, shift(@exp), "rvalue given - default scalar [$_]");
    }
}
{
    # Simple list
    my @things = (11 .. 13);
    my @exp = ('3 4 5', '11 12 13', '8 9');
    for (0, 1, 2) {
	my @list = do { given ($_) {
	    when (0) { 3 .. 5 }
	    when (2) { my $fake = 'void'; 8, 9 }
	    @things;
	} };
	is("@list", shift(@exp), "rvalue given - simple list [$_]");
    }
}
{
    # Postfix list
    my @things = (12);
    my @exp = ('3 4 5', '6 7', '12');
    for (0, 1, 2) {
	my @list = do { given ($_) {
	    3 .. 5  when 0;
	    @things when 2;
	    6, 7;
	} };
	is("@list", shift(@exp), "rvalue given - postfix list [$_]");
    }
}
{
    # Default list
    my @things = (11 .. 20); # 10 elements
    my @exp = ('m o o', '8 10', '8 10');
    for (0, 1, 2) {
	my @list = do { given ($_) {
	    when (0) { "moo" =~ /(.)/g }
	    default  { 8, scalar(@things) }
	    6, 7;
	} };
	is("@list", shift(@exp), "rvalue given - default list [$_]");
    }
}
{
    # Switch control
    my @exp = ('6 7', '', '6 7');
    for (0, 1, 2, 3) {
	my @list = do { given ($_) {
	    continue when $_ <= 1;
	    break    when 1;
	    next     when 2;
	    6, 7;
	} };
	is("@list", shift(@exp), "rvalue given - default list [$_]");
    }
}
{
    # Context propagation
    my $smart_hash = sub {
	do { given ($_[0]) {
	    'undef' when undef;
	    when ([ 1 .. 3 ]) { 1 .. 3 }
	    when (4) { my $fake; do { 4, 5 } }
	} };
    };

    my $scalar;

    $scalar = $smart_hash->();
    is($scalar, 'undef', "rvalue given - scalar context propagation [undef]");

    $scalar = $smart_hash->(4);
    is($scalar, 5,       "rvalue given - scalar context propagation [4]");

    $scalar = $smart_hash->(999);
    is($scalar, undef,   "rvalue given - scalar context propagation [999]");

    my @list;

    @list = $smart_hash->();
    is("@list", 'undef', "rvalue given - list context propagation [undef]");

    @list = $smart_hash->(2);
    is("@list", '1 2 3', "rvalue given - list context propagation [2]");

    @list = $smart_hash->(4);
    is("@list", '4 5',   "rvalue given - list context propagation [4]");

    @list = $smart_hash->(999);
    is("@list", '',      "rvalue given - list context propagation [999]");
}
{
    # Array slices
    my @list = 10 .. 15;
    my @in_list;
    my @in_slice;
    for (5, 10, 15) {
        given ($_) {
            when (@list) {
                push @in_list, $_;
                continue;
            }
            when (@list[0..2]) {
                push @in_slice, $_;
            }
        }
    }
    is("@in_list", "10 15", "when(array)");
    is("@in_slice", "10", "when(array slice)");
}
{
    # Hash slices
    my %list = map { $_ => $_ } "a" .. "f";
    my @in_list;
    my @in_slice;
    for ("a", "e", "i") {
        given ($_) {
            when (%list) {
                push @in_list, $_;
                continue;
            }
            when (@list{"a".."c"}) {
                push @in_slice, $_;
            }
        }
    }
    is("@in_list", "a e", "when(hash)");
    is("@in_slice", "a", "when(hash slice)");
}

{ # RT#84526 - Handle magical TARG
    my $x = my $y = "aaa";
    for ($x, $y) {
	given ($_) {
	    is(pos, undef, "handle magical TARG");
            pos = 1;
	}
    }
}

# Test that returned values are correctly propagated through several context
# levels (see RT #93548).
{
    my $tester = sub {
	my $id = shift;

	package fmurrr;

	our ($when_loc, $given_loc, $ext_loc);

	my $ext_lex    = 7;
	our $ext_glob  = 8;
	local $ext_loc = 9;

	given ($id) {
	    my $given_lex    = 4;
	    our $given_glob  = 5;
	    local $given_loc = 6;

	    when (0) { 0 }

	    when (1) { my $when_lex    = 1 }
	    when (2) { our $when_glob  = 2 }
	    when (3) { local $when_loc = 3 }

	    when (4) { $given_lex }
	    when (5) { $given_glob }
	    when (6) { $given_loc }

	    when (7) { $ext_lex }
	    when (8) { $ext_glob }
	    when (9) { $ext_loc }

	    'fallback';
	}
    };

    my @descriptions = qw<
	constant

	when-lexical
	when-global
	when-local

	given-lexical
	given-global
	given-local

	extern-lexical
	extern-global
	extern-local
    >;

    for my $id (0 .. 9) {
	my $desc = $descriptions[$id];

	my $res = $tester->($id);
	is $res, $id, "plain call - $desc";

	$res = do {
	    my $id_plus_1 = $id + 1;
	    given ($id_plus_1) {
		do {
		    when (/\d/) {
			--$id_plus_1;
			continue;
			456;
		    }
		};
		default {
		    $tester->($id_plus_1);
		}
		'XXX';
	    }
	};
	is $res, $id, "across continue and default - $desc";
    }
}

# Check that values returned from given/when are destroyed at the right time.
{
    {
	package Fmurrr;

	sub new {
	    bless {
		flag => \($_[1]),
		id   => $_[2],
	    }, $_[0]
	}

	sub DESTROY {
	    ${$_[0]->{flag}}++;
	}
    }

    my @descriptions = qw<
	when
	break
	continue
	default
    >;

    for my $id (0 .. 3) {
	my $desc = $descriptions[$id];

	my $destroyed = 0;
	my $res_id;

	{
	    my $res = do {
		given ($id) {
		    my $x;
		    when (0) { Fmurrr->new($destroyed, 0) }
		    when (1) { my $y = Fmurrr->new($destroyed, 1); break }
		    when (2) { $x = Fmurrr->new($destroyed, 2); continue }
		    when (2) { $x }
		    default  { Fmurrr->new($destroyed, 3) }
		}
	    };
	    $res_id = $res->{id};
	}
	$res_id = $id if $id == 1; # break doesn't return anything

	is $res_id,    $id, "given/when returns the right object - $desc";
	is $destroyed, 1,   "given/when does not leak - $desc";
    };
}

# break() must reset the stack
{
    my @res = (1, do {
	given ("x") {
	    2, 3, do {
		when (/[a-z]/) {
		    4, 5, 6, break
		}
	    }
	}
    });
    is "@res", "1", "break resets the stack";
}

# RT #94682:
# must ensure $_ is initialised and cleared at start/end of given block

{
    package RT94682;

    my $d = 0;
    sub DESTROY { $d++ };

    sub f2 {
	local $_ = 5;
	given(bless [7]) {
	    ::is($_->[0], 7, "is [7]");
	}
	::is($_, 5, "is 5");
	::is($d, 1, "DESTROY called once");
    }
    f2();
}

# check that 'when' handles all 'for' loop types

{
    my $i;

    $i = 0;
    for (1..3) {
        when (1) {$i +=    1 }
        when (2) {$i +=   10 }
        when (3) {$i +=  100 }
        default { $i += 1000 }
    }
    is($i, 111, "when in for 1..3");

    $i = 0;
    for ('a'..'c') {
        when ('a') {$i +=    1 }
        when ('b') {$i +=   10 }
        when ('c') {$i +=  100 }
        default { $i += 1000 }
    }
    is($i, 111, "when in for a..c");

    $i = 0;
    for (1,2,3) {
        when (1) {$i +=    1 }
        when (2) {$i +=   10 }
        when (3) {$i +=  100 }
        default { $i += 1000 }
    }
    is($i, 111, "when in for 1,2,3");

    $i = 0;
    my @a = (1,2,3);
    for (@a) {
        when (1) {$i +=    1 }
        when (2) {$i +=   10 }
        when (3) {$i +=  100 }
        default { $i += 1000 }
    }
    is($i, 111, 'when in for @a');
}

given("xyz") {
    no warnings "void";
    my @a = (qw(a b c), do { when(/abc/) { qw(x y) } }, qw(d e f));
    is join(",", map { $_ // "u" } @a), "a,b,c,d,e,f",
	"list value of false when";
    @a = (qw(a b c), scalar do { when(/abc/) { qw(x y) } }, qw(d e f));
    is join(",", map { $_ // "u" } @a), "a,b,c,u,d,e,f",
	"scalar value of false when";
}

# RT #133368
# index() and rindex() comparisons such as '> -1' are optimised away. Make
# sure that they're still treated as a direct boolean expression rather
# than when(X) being implicitly converted to when($_ ~~ X)

{
    my $s = "abc";
    my $ok = 0;
    given("xyz") {
        when (index($s, 'a') > -1) { $ok = 1; }
    }
    ok($ok, "RT #133368 index");

    $ok = 0;
    given("xyz") {
        when (rindex($s, 'a') > -1) { $ok = 1; }
    }
    ok($ok, "RT #133368 rindex");
}


# Okay, that'll do for now. The intricacies of the smartmatch
# semantics are tested in t/op/smartmatch.t. Taintedness of
# returned values is checked in t/op/taint.t.
__END__
