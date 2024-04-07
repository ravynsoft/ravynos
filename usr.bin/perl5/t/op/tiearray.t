#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan(tests => 75);

my %seen;

package Implement;

sub TIEARRAY {
    $seen{'TIEARRAY'}++;
    my ($class,@val) = @_;
    return bless \@val,$class;
}

sub STORESIZE {
    $seen{'STORESIZE'}++;
    my ($ob,$sz) = @_;
    return $#{$ob} = $sz-1;
}

sub EXTEND {
    $seen{'EXTEND'}++;
    my ($ob,$sz) = @_;
    return @$ob = $sz;
}

sub FETCHSIZE {
    $seen{'FETCHSIZE'}++;
    return scalar(@{$_[0]});
}

sub FETCH {
    $seen{'FETCH'}++;
    my ($ob,$id) = @_;
    return $ob->[$id];
}

sub STORE {
    $seen{'STORE'}++;
    my ($ob,$id,$val) = @_;
    $ob->[$id] = $val;
}

sub UNSHIFT {
    $seen{'UNSHIFT'}++;
    my $ob = shift;
    unshift(@$ob,@_);
}

sub PUSH {
    $seen{'PUSH'}++;
    my $ob = shift;;
    push(@$ob,@_);
}

sub CLEAR {
    $seen{'CLEAR'}++;
    @{$_[0]} = ();
}

sub DESTROY {
    $seen{'DESTROY'}++;
}

sub POP {
    $seen{'POP'}++;
    my ($ob) = @_;
    return pop(@$ob);
}

sub SHIFT {
    $seen{'SHIFT'}++;
    my ($ob) = @_;
    return shift(@$ob);
}

sub SPLICE {
    $seen{'SPLICE'}++;
    my $ob  = shift;
    my $off = @_ ? shift : 0;
    my $len = @_ ? shift : @$ob-1;
    return splice(@$ob,$off,$len,@_);
}

package NegIndex;               # 20020220 MJD
@ISA = 'Implement';

# simulate indices -2 .. 2
my $offset = 2;
$NegIndex::NEGATIVE_INDICES = 1;

sub FETCH {
    my ($ob,$id) = @_;
    #print "# FETCH @_\n";
    $id += $offset;
    $ob->[$id];
}

sub STORE {
    my ($ob,$id,$value) = @_;
    #print "# STORE @_\n";
    $id += $offset;
    $ob->[$id] = $value;
}

sub DELETE {
    my ($ob,$id) = @_;
    #print "# DELETE @_\n";
    $id += $offset;
    delete $ob->[$id];
}

sub EXISTS {
    my ($ob,$id) = @_;
    #print "# EXISTS @_\n";
    $id += $offset;
    exists $ob->[$id];
}

#
# Returning -1 from FETCHSIZE used to get casted to U32 causing a
# segfault
#

package NegFetchsize;

sub TIEARRAY  { bless [] }
sub FETCH     { }
sub FETCHSIZE { -1 }


package main;

{
    $seen{'DESTROY'} = 0;
    my @ary;

    {
        my $ob = tie @ary,'Implement',3,2,1;
        ok($ob);
        is(tied(@ary), $ob);
    }

    is(@ary, 3);
    is($#ary, 2);
    is(join(':',@ary), '3:2:1');
    cmp_ok($seen{'FETCH'}, '>=', 3);

    @ary = (1,2,3);

    cmp_ok($seen{'STORE'}, '>=', 3);
    is(join(':',@ary), '1:2:3');

    {
        my @thing = @ary;
        is(join(':',@thing), '1:2:3');

        tie @thing,'Implement';
        @thing = @ary;
        is(join(':',@thing), '1:2:3');
    }
    is($seen{'DESTROY'}, 1, "thing freed");

    is(pop(@ary), 3);
    is($seen{'POP'}, 1);
    is(join(':',@ary), '1:2');

    is(push(@ary,4), 3);
    is($seen{'PUSH'}, 1);
    is(join(':',@ary), '1:2:4');

    my @x = splice(@ary,1,1,7);

    is($seen{'SPLICE'}, 1);
    is(@x, 1);
    is($x[0], 2);
    is(join(':',@ary), '1:7:4');

    is(shift(@ary), 1);
    is($seen{'SHIFT'}, 1);
    is(join(':',@ary), '7:4');

    my $n = unshift(@ary,5,6);
    is($seen{'UNSHIFT'}, 1);
    is($n, 4);
    is(join(':',@ary), '5:6:7:4');

    @ary = split(/:/,'1:2:3');
    is(join(':',@ary), '1:2:3');

    my $t = 0;
    foreach $n (@ary) {
         is($n, ++$t);
    }

    # (30-33) 20020303 mjd-perl-patch+@plover.com
    @ary = ();
    $seen{POP} = 0;
    pop @ary;                       # this didn't used to call POP at all
    is($seen{POP}, 1);
    $seen{SHIFT} = 0;
    shift @ary;                     # this didn't used to call SHIFT at  all
    is($seen{SHIFT}, 1);
    $seen{PUSH} = 0;
    my $got = push @ary;            # this didn't used to call PUSH at all
    is($got, 0);
    is($seen{PUSH}, 1);
    $seen{UNSHIFT} = 0;
    $got = unshift @ary;            # this didn't used to call UNSHIFT at all
    is($got, 0);
    is($seen{UNSHIFT}, 1);

    @ary = qw(3 2 1);
    is(join(':',@ary), '3:2:1');

    $#ary = 1;
    is($seen{'STORESIZE'}, 1, 'seen STORESIZE');
    is(join(':',@ary), '3:2');

    sub arysize :lvalue { $#ary }
    arysize()--;
    is($seen{'STORESIZE'}, 2, 'seen STORESIZE');
    is(join(':',@ary), '3');

    untie @ary;
}
is($seen{'DESTROY'}, 2, "ary freed");

# 20020401 mjd-perl-patch+@plover.com
# Thanks to Dave Mitchell for the small test case and the fix
{
    my @a;

    sub X::TIEARRAY { bless {}, 'X' }

    sub X::SPLICE {
        do '/dev/null';
        die;
    }

    tie @a, 'X';
    eval { splice(@a) };
    # If we survived this far.
    pass();
}

# 20020220 mjd-perl-patch+@plover.com
{
    $seen{'DESTROY'} = 0;

    my @n;
    tie @n => 'NegIndex', ('A' .. 'E');

    # FETCH
    is($n[0], 'C');
    is($n[1], 'D');
    is($n[2], 'E');
    is($n[-1], 'B');
    is($n[-2], 'A');

    # STORE
    $n[-2] = 'a';
    is($n[-2], 'a');
    $n[-1] = 'b';
    is($n[-1], 'b');
    $n[0] = 'c';
    is($n[0], 'c');
    $n[1] = 'd';
    is($n[1], 'd');
    $n[2] = 'e';
    is($n[2], 'e');

    # DELETE and EXISTS
    for (-2 .. 2) {
        ok($n[$_]);
        delete $n[$_];
        is(defined($n[$_]), '');
        is(exists($n[$_]), '');
    }
}
is($seen{'DESTROY'}, 1, "n freed");

{
    tie my @dummy, "NegFetchsize";
    eval { "@dummy"; };
    like($@, qr/^FETCHSIZE returned a negative value/,
	 " - croak on negative FETCHSIZE");
}

{
    # check that a tied element assigned to an array doesn't remain tied

    package Magical;

    my $i = 10;

    sub TIEARRAY { bless [1] }
    sub TIEHASH  { bless [1] }
    sub FETCHSIZE { 1; }
    sub FETCH { $i++ }
    sub STORE { $_[0][0] = $_[1]; }
    sub FIRSTKEY { 0 }
    sub NEXTKEY { }

    package main;

    my (@a, @b);
    tie @a, 'Magical';
    @b = @a;
    is ($b[0],  10, "Magical array fetch 1");
    $b[0] = 100;
    is ($b[0], 100, "Magical array fetch 2");

    my (%a, %b);
    tie %a, 'Magical';
    %b = %a;
    is ($b{0},  11, "Magical hash fetch 1");
    $b{0} = 100;
    is ($b{0}, 100, "Magical hash fetch 2");
}
