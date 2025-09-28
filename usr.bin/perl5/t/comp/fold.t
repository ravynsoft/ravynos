#!./perl -w

# Uncomment this for testing, but don't leave it in for "production", as
# we've not yet verified that use works.
# use strict;

print "1..35\n";
my $test = 0;

# Historically constant folding was performed by evaluating the ops, and if
# they threw an exception compilation failed. This was seen as buggy, because
# even illegal constants in unreachable code would cause failure. So now
# illegal expressions are reported at runtime, if the expression is reached,
# making constant folding consistent with many other languages, and purely an
# optimisation rather than a behaviour change.


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

sub like {
    my ($got, $pattern, $name) = @_;
    $test = $test + 1;
    if (defined $got && $got =~ $pattern) {
	print "ok $test - $name\n";
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
	print "ok $test - $name\n";
	return 1;
    }
    failed($got, "'$expect'", $name);
}

sub ok {
    my ($got, $name) = @_;
    $test = $test + 1;
    if ($got) {
	print "ok $test - $name\n";
	return 1;
    }
    failed($got, "a true value", $name);
}

my $a;
$a = eval '$b = 0/0 if 0; 3';
is ($a, 3, 'constants in conditionals don\'t affect constant folding');
is ($@, '', 'no error');

my $b = 0;
$a = eval 'if ($b) {return sqrt -3} 3';
is ($a, 3, 'variables in conditionals don\'t affect constant folding');
is ($@, '', 'no error');

$a = eval q{
	$b = eval q{if ($b) {return log 0} 4};
 	is ($b, 4, 'inner eval folds constant');
	is ($@, '', 'no error');
	5;
};
is ($a, 5, 'outer eval folds constant');
is ($@, '', 'no error');

# warn and die hooks should be disabled during constant folding

{
    my $c = 0;
    local $SIG{__WARN__} = sub { $c++   };
    local $SIG{__DIE__}  = sub { $c+= 2 };
    eval q{
	is($c, 0, "premature warn/die: $c");
	my $x = "a"+5;
	is($c, 1, "missing warn hook");
	is($x, 5, "a+5");
	$c = 0;
	$x = 1/0;
    };
    like ($@, qr/division/, "eval caught division");
    is($c, 2, "missing die hook");
}

# [perl #20444] Constant folding should not change the meaning of match
# operators.
{
 local *_;
 $_="foo"; my $jing = 1;
 ok scalar $jing =~ (1 ? /foo/ : /bar/),
   'lone m// is not bound via =~ after ? : folding';
 ok scalar $jing =~ (0 || /foo/),
   'lone m// is not bound via =~ after || folding';
 ok scalar $jing =~ (1 ? s/foo/foo/ : /bar/),
   'lone s/// is not bound via =~ after ? : folding';
 ok scalar $jing =~ (0 || s/foo/foo/),
   'lone s/// is not bound via =~ after || folding';
 $jing = 3;
 ok scalar $jing =~ (1 ? y/fo// : /bar/),
   'lone y/// is not bound via =~ after ? : folding';
 ok scalar $jing =~ (0 || y/fo//),
   'lone y/// is not bound via =~ after || folding';
}

# [perl #78064] or print
package other { # hide the "ok" sub
 BEGIN { $^W = 0 }
 print 0 ? not_ok : ok;
 print " ", ++$test, " - print followed by const ? BEAR : BEAR\n";
 print 1 ? ok : not_ok;
 print " ", ++$test, " - print followed by const ? BEAR : BEAR (again)\n";
 print 1 && ok;
 print " ", ++$test, " - print followed by const && BEAR\n";
 print 0 || ok;
 print " ", ++$test, " - print followed by const || URSINE\n";
 BEGIN { $^W = 1 }
}

# or stat
print "not " unless stat(1 ? INSTALL : 0) eq stat("INSTALL");
print "ok ", ++$test, " - stat(const ? word : ....)\n";
# in case we are in t/
print "not " unless stat(1 ? TEST : 0) eq stat("TEST");
print "ok ", ++$test, " - stat(const ? word : ....)\n";

# or truncate
my $n = "for_fold_dot_t$$";
open F, ">$n" or die "open: $!";
print F "bralh blah blah \n";
close F or die "close $!";
eval "truncate 1 ? $n : 0, 0;";
print "not " unless -z $n;
print "ok ", ++$test, " - truncate(const ? word : ...)\n";
unlink $n;

# Constant folding should not change the mutability of returned values.
for(1+2) {
    eval { $_++ };
    print "not " unless $_ eq 4;
    print "ok ", ++$test,
          " - 1+2 returns mutable value, just like \$a+\$b",
          "\n";
}

# [perl #119055]
# We hide the implementation detail that qq "foo" is implemented using
# constant folding.
eval { ${\"hello\n"}++ };
print "not " unless $@ =~ "Modification of a read-only value attempted at";
print "ok ", ++$test, " - qq with no vars is a constant\n";

# [perl #119501]
my @values;
for (1,2) { for (\(1+3)) { push @values, $$_; $$_++ } }
is "@values", "4 4",
   '\1+3 folding making modification affect future retvals';

{
    BEGIN { $^W = 0; $::{u} = \undef }
    my $w;
    local $SIG{__WARN__} = sub { ++$w };
    () = 1 + u;
    is $w, 1, '1+undef_constant is not folded outside warninsg scope';
    BEGIN { $^W = 1 }
}

$a = eval 'my @z; @z = 0..~0 if 0; 3';
is ($a, 3, "list constant folding doesn't signal compile-time error");
is ($@, '', 'no error');

$b = 0;
$a = eval 'my @z; @z = 0..~0 if $b; 3';
is ($a, 3, "list constant folding doesn't signal compile-time error");
is ($@, '', 'no error');

$a = eval 'local $SIG{__WARN__} = sub {}; join("", ":".."~", "z")';
is ($a, ":z", "aborted list constant folding still executable");
