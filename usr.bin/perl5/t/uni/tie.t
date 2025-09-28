#!perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}

plan (tests => 10);
use strict;

{
    package UTF8Toggle;

    sub TIESCALAR {
	my $class = shift;
	my $value = shift;
	my $state = shift||0;
	return bless [$value, $state], $class;
    }

    sub FETCH {
	my $self = shift;
	$self->[1] = ! $self->[1];
	if ($self->[1]) {
	    utf8::downgrade($self->[0]);
	} else {
	    utf8::upgrade($self->[0]);
	}
	$self->[0];
    }
}

foreach my $t ("ASCII", "B\366se") {
    my $length = length $t;

    my $u;
    tie $u, 'UTF8Toggle',  $t;
    is (length $u, $length, "length of '$t'");
    is (length $u, $length, "length of '$t'");
    is (length $u, $length, "length of '$t'");
    is (length $u, $length, "length of '$t'");
}

{
    use utf8;
    use open qw( :utf8 :std );
    package Tìè::UTF8 {
        sub TIESCALAR {
            return bless {}, shift;
        }
    }
    
    my $t;
    tie $t, 'Tìè::UTF8';
    is ref(tied($t)), 'Tìè::UTF8', "Tie'ing to a UTF8 package works.";
}
{
    local $::TODO = "Need more tests!";
    fail();
}
