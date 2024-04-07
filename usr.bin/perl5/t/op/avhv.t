#!./perl

# This test was originally for pseudo-hashes.  It now exists to ensure
# they were properly removed in 5.9.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

require Tie::Array;

package Tie::BasicArray;
@ISA = 'Tie::Array';
sub TIEARRAY  { bless [], $_[0] }
sub STORE     { $_[0]->[$_[1]] = $_[2] }
sub FETCH     { $_[0]->[$_[1]] }
sub FETCHSIZE { scalar(@{$_[0]})} 
sub STORESIZE { $#{$_[0]} = $_[1]+1 }

package main;

plan(tests => 40);

# Helper function to check the typical error message.
sub not_hash {
    my($err) = shift;
    like( $err, qr/^Not a HASH reference / ) ||
      printf STDERR "# at %s line %d.\n", (caller)[1,2];
}

# Something to place inside if blocks and while loops that won't get
# compiled out.
my $foo = 42;
sub no_op { $foo++ }


$sch = {
    'abc' => 1,
    'def' => 2,
    'jkl' => 3,
};

# basic normal array
$a = [];
$a->[0] = $sch;

eval {
    $a->{'abc'} = 'ABC';
};
not_hash($@);

eval {
    $a->{'def'} = 'DEF';
};
not_hash($@);

eval {
    $a->{'jkl'} = 'JKL';
};
not_hash($@);

eval {
    @keys = keys %$a;
};
not_hash($@);

eval {
    @values = values %$a;
};
not_hash($@);

eval {
    while( my($k,$v) = each %$a ) {
        no_op;
    }
};
not_hash($@);


# quick check with tied array
tie @fake, 'Tie::StdArray';
$a = \@fake;
$a->[0] = $sch;

eval {
    $a->{'abc'} = 'ABC';
};
not_hash($@);

eval {
    if ($a->{'abc'} eq 'ABC') { no_op(23) } else { no_op(42) }
};
not_hash($@);

# quick check with tied array
tie @fake, 'Tie::BasicArray';
$a = \@fake;
$a->[0] = $sch;

eval {
    $a->{'abc'} = 'ABC';
};
not_hash($@);

eval {
    if ($a->{'abc'} eq 'ABC') { no_op(23) } else { no_op(42) }
};
not_hash($@);

# quick check with tied array & tied hash
require Tie::Hash;
tie %fake, Tie::StdHash;
%fake = %$sch;
$a->[0] = \%fake;

eval {
    $a->{'abc'} = 'ABC';
};
not_hash($@);

eval {
    if ($a->{'abc'} eq 'ABC') { no_op(23) } else { no_op(42) }
};
not_hash($@);


# hash slice
eval {
    my $slice = join('', 'x',@$a{'abc','def'},'x');
};
not_hash($@);


# evaluation in scalar context
my $avhv = [{}];

eval {
    () = %$avhv;
};
not_hash($@);

push @$avhv, "a";
eval {
    () = %$avhv;
};
not_hash($@);

$avhv = [];
eval { $a = %$avhv };
not_hash($@);

$avhv = [{foo=>1, bar=>2}];
eval {
    %$avhv =~ m,^\d+/\d+,;
};
not_hash($@);

# check if defelem magic works
sub f {
    print "not " unless $_[0] eq 'a';
    $_[0] = 'b';
    print "ok 11\n";
}
$a = [{key => 1}, 'a'];
eval {
    f($a->{key});
};
not_hash($@);

# check if exists() is behaving properly
$avhv = [{foo=>1,bar=>2,pants=>3}];
eval {
    no_op if exists $avhv->{bar};
};
not_hash($@);

eval {
    $avhv->{pants} = undef;
};
not_hash($@);

eval {
    no_op if exists $avhv->{pants};
};
not_hash($@);

eval {
    no_op if exists $avhv->{bar};
};
not_hash($@);

eval {
    $avhv->{bar} = 10;
};
not_hash($@);

eval {
    no_op unless exists $avhv->{bar} and $avhv->{bar} == 10;
};
not_hash($@);

eval {
    $v = delete $avhv->{bar};
};
not_hash($@);

eval {
    no_op if exists $avhv->{bar};
};
not_hash($@);

eval {
    $avhv->{foo} = 'xxx';
};
not_hash($@);
eval {
    $avhv->{bar} = 'yyy';
};
not_hash($@);
eval {
    $avhv->{pants} = 'zzz';
};
not_hash($@);
eval {
    @x = delete @{$avhv}{'foo','pants'};
};
not_hash($@);
eval {
    no_op unless "$avhv->{bar}" eq "yyy";
};
not_hash($@);

# hash assignment
eval {
    %$avhv = ();
};
not_hash($@);

eval {
    %hv = %$avhv;
};
not_hash($@);

eval {
    %$avhv = (foo => 29, pants => 2, bar => 0);
};
not_hash($@);

my $extra;
my @extra;
eval {
    ($extra, %$avhv) = ("moo", foo => 42, pants => 53, bar => "HIKE!");
};
not_hash($@);

eval {
    %$avhv = ();
    (%$avhv, $extra) = (foo => 42, pants => 53, bar => "HIKE!");
};
not_hash($@);

eval {
    @extra = qw(whatever and stuff);
    %$avhv = ();
};
not_hash($@);
eval {
    (%$avhv, @extra) = (foo => 42, pants => 53, bar => "HIKE!");
};
not_hash($@);

eval {
    (@extra, %$avhv) = (foo => 42, pants => 53, bar => "HIKE!");
};
not_hash($@);

# Check hash slices (BUG ID 20010423.002 (#6879))
$avhv = [{foo=>1, bar=>2}];
eval {
    @$avhv{"foo", "bar"} = (42, 53);
};
not_hash($@);
