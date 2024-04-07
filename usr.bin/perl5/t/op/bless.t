#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan (118);
# Please do not eliminate the plan.  We have tests in DESTROY blocks.

sub expected {
    my($object, $package, $type) = @_;
    print "# $object $package $type\n";
    is(ref($object), $package);
    my $r = qr/^\Q$package\E=(\w+)\(0x([0-9a-f]+)\)$/;
    like("$object", $r);
    if ("$object" =~ $r) {
	is($1, $type);
	# in 64-bit platforms hex warns for 32+ -bit values
	cmp_ok(do {no warnings 'portable'; hex($2)}, '==', $object);
    }
    else {
	fail(); fail();
    }
}

# test blessing simple types

$a1 = bless {}, "A";
expected($a1, "A", "HASH");
$b1 = bless [], "B";
expected($b1, "B", "ARRAY");
$c1 = bless \(map "$_", "test"), "C";
expected($c1, "C", "SCALAR");
our $test = "foo"; $d1 = bless \*test, "D";
expected($d1, "D", "GLOB");
$e1 = bless sub { 1 }, "E";
expected($e1, "E", "CODE");
$f1 = bless \[], "F";
expected($f1, "F", "REF");
$g1 = bless \substr("test", 1, 2), "G";
expected($g1, "G", "LVALUE");

# blessing ref to object doesn't modify object

expected(bless(\$a1, "F"), "F", "REF");
expected($a1, "A", "HASH");

# reblessing does modify object

bless $a1, "A2";
expected($a1, "A2", "HASH");

# local and my
{
    local $a1 = bless $a1, "A3";	# should rebless outer $a1
    local $b1 = bless [], "B3";
    my $c1 = bless $c1, "C3";		# should rebless outer $c1
    our $test2 = ""; my $d1 = bless \*test2, "D3";
    expected($a1, "A3", "HASH");
    expected($b1, "B3", "ARRAY");
    expected($c1, "C3", "SCALAR");
    expected($d1, "D3", "GLOB");
}
expected($a1, "A3", "HASH");
expected($b1, "B", "ARRAY");
expected($c1, "C3", "SCALAR");
expected($d1, "D", "GLOB");

# class is magic
"E" =~ /(.)/;
expected(bless({}, $1), "E", "HASH");
{
    local $! = 1;
    my $string = "$!";
    $! = 2;	# attempt to avoid cached string
    $! = 1;
    expected(bless({}, $!), $string, "HASH");

# ref is ref to magic
    {
	{
	    package F;
	    sub test { main::is(${$_[0]}, $string) }
	}
	$! = 2;
	$f1 = bless \$!, "F";
	$! = 1;
	$f1->test;
    }
}

# ref is magic
### example of magic variable that is a reference??

# no class, or empty string (with a warning), or undef (with two)
expected(bless([]), 'main', "ARRAY");
{
    local $SIG{__WARN__} = sub { push @w, join '', @_ };
    use warnings;

    $m = bless [];
    expected($m, 'main', "ARRAY");
    is (scalar @w, 0);

    @w = ();
    $m = bless [], '';
    expected($m, 'main', "ARRAY");
    is (scalar @w, 1);

    @w = ();
    $m = bless [], undef;
    expected($m, 'main', "ARRAY");
    is (scalar @w, 2);
}

# class is a ref
$a1 = bless {}, "A4";
$b1 = eval { bless {}, $a1 };
like ($@, qr/^Attempt to bless into a reference at /, "class is a ref");

# class is an overloaded ref
{
    package H4;
    use overload '""' => sub { "C4" };
}
$h1 = bless {}, "H4";
$c4 = eval { bless \$test, $h1 };
is ($@, '', "class is an overloaded ref");
expected($c4, 'C4', "SCALAR");

{
    my %h = 1..2;
    my($k) = keys %h; 
    my $x=\$k;
    bless $x, 'pam';
    is(ref $x, 'pam');

    my $a = bless \(keys %h), 'zap';
    is(ref $a, 'zap');
}

bless [], "main::";
ok(1, 'blessing into main:: does not crash'); # [perl #87388]

sub _117941 { package _117941; bless [] }
delete $::{"_117941::"};
eval { _117941() };
like $@, qr/^Attempt to bless into a freed package at /,
        'bless with one arg when current stash is freed';

for(__PACKAGE__) {
    eval { bless \$_ };
    like $@, qr/^Modification of a read-only value attempted/,
         'read-only COWs cannot be blessed';
}

sub TIESCALAR { bless \(my $thing = pop), shift }
sub FETCH { ${$_[0]} }
tie $tied, main => $untied = [];
eval { bless $tied };
is ref $untied, "main", 'blessing through tied refs' or diag $@;

bless \$victim, "Food";
eval 'bless \$Food::bard, "Bard"';
sub Bard::DESTROY {
    isnt ref(\$victim), '__ANON__',
        'reblessing does not leave an object in limbo temporarily';
    bless \$victim
}
undef *Food::;
{
    my $w;
    # This should catch ‘Attempt to free unreferenced scalar’.
    local $SIG{__WARN__} = sub { $w .= shift };
    bless \$victim;
    is $w, undef,
       'no warnings when reblessing inside DESTROY triggered by reblessing'
}

TODO: {
    my $ref;
    sub new {
        my ($class, $code) = @_;
        my $ret = ref($code);
        bless $code => $class;
        return $ret;
    }
    for my $i (1 .. 2) {
        $ref = main -> new (sub {$i});
    }
    is $ref, 'CODE', 'RT #3305: Code ref should not be blessed yet';

    local $TODO = 'RT #3305';

    for my $i (1 .. 2) {
        $ref = main -> new (sub {});
    }
    is $ref, 'CODE', 'RT #3305: Code ref should not be blessed yet';
}

my $t_3306_c = 0;
my $t_3306_s = 0;

{
    sub FooClosure::new {
        my ($class, $code) = @_;
        bless $code => $class;
    }
    sub FooClosure::DESTROY {
        $t_3306_c++;
    }

    sub FooSub::new {
        my ($class, $code) = @_;
        bless $code => $class;
    }
    sub FooSub::DESTROY {
        $t_3306_s++;
    }

    my $i = '';
    FooClosure -> new (sub {$i});
    FooSub -> new (sub {});
}

is $t_3306_c, 1, 'RT #3306: DESTROY should be called on CODE ref (works on closures)';

TODO: {
    local $TODO = 'RT #3306';
    is $t_3306_s, 1, 'RT #3306: DESTROY should be called on CODE ref';
}

undef *FooClosure::;
undef *FooSub::;
