#!./perl

use Test::More tests => 16;

use Storable ();

use warnings;
use strict;

package TEST;

sub make {
	my $pkg = shift;
	return bless { a => 1, b => 2 }, $pkg;
}

package TIED_HASH;

sub TIEHASH {
	my $pkg = shift;
	return bless { a => 1, b => 2 }, $pkg;
}

sub FETCH {
	my ($self, $key) = @_;
	return $self->{$key};
}

sub STORE {
	my ($self, $key, $value) = @_;
	$self->{$key} = $value;
}

sub FIRSTKEY {
	my $self = shift;
	keys %$self;
	return each %$self;
}

sub NEXTKEY {
	my $self = shift;
	return each %{$self};
}

sub EXISTS {
	my ($self, $key) = @_;
	return exists $self->{$key};
}

package main;

{
	my $obj = TEST->make;

	is_deeply($obj, { a => 1, b => 2 }, "object contains correct data");

	my $frozen = Storable::freeze($obj);
	my ($t1, $t2) = Storable::thaw($frozen);

	{
		no warnings 'once';
		local $Storable::flags = Storable::FLAGS_COMPAT();
		$t2 = Storable::thaw($frozen);
	}

	is_deeply($t1, $t2, "objects contain matching data");
	is(ref $t1, 'TEST', "default object is blessed");
	is(ref $t2, 'TEST', "compat object is blessed into correct class");

	my $t3 = Storable::thaw($frozen, Storable::FLAGS_COMPAT());
	is_deeply($t2, $t3, "objects contain matching data (explicit test)");
	is(ref $t3, 'TEST', "compat object is blessed into correct class (explicit test)");

	my $t4 = Storable::thaw($frozen, Storable::BLESS_OK());
	is_deeply($t2, $t3, "objects contain matching data (explicit test for bless)");
	is(ref $t3, 'TEST', "compat object is blessed into correct class (explicit test for bless)");

	{
		no warnings 'once';
		local $Storable::flags = Storable::FLAGS_COMPAT();
		my $t5 = Storable::thaw($frozen, 0);
		my $t6 = Storable::thaw($frozen, Storable::TIE_OK());

		is_deeply($t1, $t5, "objects contain matching data");
		is_deeply($t1, $t6, "objects contain matching data for TIE_OK");
		is(ref $t5, 'HASH', "default object is unblessed");
		is(ref $t6, 'HASH', "TIE_OK object is unblessed");
	}
}

{
	tie my %hash, 'TIED_HASH';
	ok(tied %hash, "hash is tied");
	my $obj = { bow => \%hash };

	my $frozen = Storable::freeze($obj);
	my $t1 = Storable::thaw($frozen, Storable::FLAGS_COMPAT());
	my $t2 = eval { Storable::thaw($frozen); };

	ok(!$@, "trying to thaw a tied value succeeds");
	ok(tied %{$t1->{bow}}, "compat object is tied");
	is(ref tied %{$t1->{bow}}, 'TIED_HASH', "compat object is tied into correct class");
}
