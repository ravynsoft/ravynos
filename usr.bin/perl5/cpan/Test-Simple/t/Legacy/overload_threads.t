#!perl -w
use Test2::Util qw/CAN_THREAD/;

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}
chdir 't';

BEGIN {
    # There was a bug with overloaded objects and threads.
    # See rt.cpan.org 4218
    eval { require threads; 'threads'->import; 1; } if CAN_THREAD;
}

use Test::More;

plan skip_all => "known to crash on $]" if $] le "5.006002";

plan tests => 5;


package Overloaded;

use overload
  q{""} => sub { $_[0]->{string} };

sub new {
    my $class = shift;
    bless { string => shift }, $class;
}


package main;

my $warnings = '';
local $SIG{__WARN__} = sub { $warnings = join '', @_ };

# overloaded object as name
my $obj = Overloaded->new('foo');
ok( 1, $obj );

# overloaded object which returns undef as name
my $undef = Overloaded->new(undef);
pass( $undef );

is( $warnings, '' );


TODO: {
    my $obj = Overloaded->new('not really todo, testing overloaded reason');
    local $TODO = $obj;
    fail("Just checking todo as an overloaded value");
}


SKIP: {
    my $obj = Overloaded->new('not really skipped, testing overloaded reason');
    skip $obj, 1;
}
