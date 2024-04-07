#!./perl

# Tests for @{^COMPILE_SCOPE_CONTAINER}

use strict;
use warnings;
use Test::More tests => 12;
use XS::APItest;

BEGIN { 
    # this has to be a full glob alias, since the GvAV gets replaced
    *COMPILE_SCOPE_CONTAINER = \*XS::APItest::COMPILE_SCOPE_CONTAINER;
}
our @COMPILE_SCOPE_CONTAINER;

my %destroyed;

BEGIN {
    package CounterObject;

    sub new {
        my ($class, $name) = @_;
        return bless { name => $name }, $class;
    }

    sub name {
        my ($self) = @_;
        return $self->{name};
    }

    sub DESTROY {
        my ($self) = @_;
        $destroyed{ $self->name }++;
    }


    package ReplaceCounter;
    $INC{'ReplaceCounter.pm'} = __FILE__;

    sub import {
        my ($self, $counter) = @_;
        $COMPILE_SCOPE_CONTAINER[-1] = CounterObject->new($counter);
    }

    package InstallCounter;
    $INC{'InstallCounter.pm'} = __FILE__;

    sub import {
        my ($class, $counter) = @_;
        push @COMPILE_SCOPE_CONTAINER, CounterObject->new($counter);
    }

    package TestCounter;
    $INC{'TestCounter.pm'} = __FILE__;

    sub import {
        my ($class, $counter, $number, $message) = @_;

        $number = 1
            unless defined $number;
        $message = "counter $counter is found $number times"
            unless defined $message;

        ::is scalar(grep { $_->name eq $counter } @{COMPILE_SCOPE_CONTAINER}),
            $number,
            $message;
    }
}

{
    use InstallCounter 'root';
    use InstallCounter '3rd-party';

    {
        BEGIN { ok(!keys %destroyed, 'nothing destroyed yet'); }

        use ReplaceCounter 'replace';

        BEGIN { ok(!keys %destroyed, 'nothing destroyed yet'); }

        use TestCounter '3rd-party', 0, '3rd-party no longer visible';
        use TestCounter 'replace',   1, 'replacement now visible';
        use TestCounter 'root';

        BEGIN { ok(!keys %destroyed, 'nothing destroyed yet'); }
    }

    BEGIN {
        ok $destroyed{replace}, 'replacement has been destroyed after end of outer scope';
    }

    use TestCounter 'root',     1, 'root visible again';
    use TestCounter 'replace',  0, 'lower replacement no longer visible';
    use TestCounter '3rd-party';
}

ok $destroyed{ $_ }, "$_ has been destroyed after end of outer scope"
    for 'root', '3rd-party';
