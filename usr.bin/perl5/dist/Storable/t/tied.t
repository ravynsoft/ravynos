#!./perl
#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
    require 'st-dump.pl';
}

use Storable qw(freeze thaw);
$Storable::flags = Storable::FLAGS_COMPAT;

use Test::More tests => 25;

($scalar_fetch, $array_fetch, $hash_fetch) = (0, 0, 0);

package TIED_HASH;

sub TIEHASH {
	my $self = bless {}, shift;
	return $self;
}

sub FETCH {
	my $self = shift;
	my ($key) = @_;
	$main::hash_fetch++;
	return $self->{$key};
}

sub STORE {
	my $self = shift;
	my ($key, $value) = @_;
	$self->{$key} = $value;
}

sub FIRSTKEY {
	my $self = shift;
	scalar keys %{$self};
	return each %{$self};
}

sub NEXTKEY {
	my $self = shift;
	return each %{$self};
}

package TIED_ARRAY;

sub TIEARRAY {
	my $self = bless [], shift;
	return $self;
}

sub FETCH {
	my $self = shift;
	my ($idx) = @_;
	$main::array_fetch++;
	return $self->[$idx];
}

sub STORE {
	my $self = shift;
	my ($idx, $value) = @_;
	$self->[$idx] = $value;
}

sub FETCHSIZE {
	my $self = shift;
	return @{$self};
}

package TIED_SCALAR;

sub TIESCALAR {
	my $scalar;
	my $self = bless \$scalar, shift;
	return $self;
}

sub FETCH {
	my $self = shift;
	$main::scalar_fetch++;
	return $$self;
}

sub STORE {
	my $self = shift;
	my ($value) = @_;
	$$self = $value;
}

package FAULT;

$fault = 0;

sub TIESCALAR {
	my $pkg = shift;
	return bless [@_], $pkg;
}

sub FETCH {
	my $self = shift;
	my ($href, $key) = @$self;
	$fault++;
	untie $href->{$key};
	return $href->{$key} = 1;
}

package main;

$a = 'toto';
$b = \$a;

$c = tie %hash, TIED_HASH;
$d = tie @array, TIED_ARRAY;
tie $scalar, TIED_SCALAR;

#$scalar = 'foo';
#$hash{'attribute'} = \$d;
#$array[0] = $c;
#$array[1] = \$scalar;

### If I say
###   $hash{'attribute'} = $d;
### below, then dump() incorrectly dumps the hash value as a string the second
### time it is reached. I have not investigated enough to tell whether it's
### a bug in my dump() routine or in the Perl tieing mechanism.
$scalar = 'foo';
$hash{'attribute'} = 'plain value';
$array[0] = \$scalar;
$array[1] = $c;
$array[2] = \@array;

@tied = (\$scalar, \@array, \%hash);
%a = ('key', 'value', 1, 0, $a, $b, 'cvar', \$a, 'scalarref', \$scalar);
@a = ('first', 3, -4, -3.14159, 456, 4.5, $d, \$d,
	$b, \$a, $a, $c, \$c, \%a, \@array, \%hash, \@tied);

my $f = freeze(\@a);
isnt($f, undef);

$dumped = &dump(\@a);
isnt($dumped, undef);

$root = thaw($f);
isnt($root, undef);

$got = &dump($root);
isnt($got, undef);

### Used to see the manifestation of the bug documented above.
### print "original: $dumped";
### print "--------\n";
### print "got: $got";
### print "--------\n";

is($got, $dumped);

$g = freeze($root);
is(length $f, length $g);

# Ensure the tied items in the retrieved image work
@old = ($scalar_fetch, $array_fetch, $hash_fetch);
@tied = ($tscalar, $tarray, $thash) = @{$root->[$#{$root}]};
@type = qw(SCALAR  ARRAY  HASH);

is(ref tied $$tscalar, 'TIED_SCALAR');
is(ref tied @$tarray, 'TIED_ARRAY');
is(ref tied %$thash, 'TIED_HASH');

@new = ($$tscalar, $tarray->[0], $thash->{'attribute'});
@new = ($scalar_fetch, $array_fetch, $hash_fetch);

# Tests 10..15
for ($i = 0; $i < @new; $i++) {
	is($new[$i], $old[$i] + 1);
	is(ref $tied[$i], $type[$i]);
}

# Check undef ties
my $h = {};
tie $h->{'x'}, 'FAULT', $h, 'x';
my $hf = freeze($h);
isnt($hf, undef);
is($FAULT::fault, 0);
is($h->{'x'}, 1);
is($FAULT::fault, 1);

my $ht = thaw($hf);
isnt($ht, undef);
is($ht->{'x'}, 1);
is($FAULT::fault, 2);

{
    package P;
    use Storable qw(freeze thaw);
    our ($a, $b);
    $b = "not ok ";
    sub TIESCALAR { bless \$a } sub FETCH { "ok " }
    tie $a, P; my $r = thaw freeze \$a; $b = $$r;
    main::is($b, "ok ");
}

{
    # blessed ref to tied object should be thawed blessed
    my @a;
    tie @a, TIED_ARRAY;
    my $r = bless \@a, 'FOO99';
    my $f = freeze($r);
    my $t = thaw($f);
    isnt($t, undef);
    like("$t", qr/^FOO99=ARRAY/);
}
