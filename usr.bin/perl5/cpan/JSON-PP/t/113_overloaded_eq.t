use strict;
use warnings;
use Test::More tests => 4;

BEGIN {
    $ENV{ PERL_JSON_BACKEND } = 0;
}

use JSON::PP;

my $json = JSON::PP->new->convert_blessed;

my $obj = OverloadedObject->new( 'foo' );
ok( $obj eq 'foo' );
is( $json->encode( [ $obj ] ), q{["foo"]} );

# rt.cpan.org #64783
my $foo  = bless {}, 'Foo';
my $bar  = bless {}, 'Bar';

eval q{ $json->encode( $foo ) };
ok($@);
eval q{ $json->encode( $bar ) };
ok(!$@);


package Foo;

use strict;
use warnings;
use overload (
    'eq' => sub { 0 },
    '""' => sub { $_[0] },
    fallback => 1,
);

sub TO_JSON {
    return $_[0];
}

package Bar;

use strict;
use warnings;
use overload (
    'eq' => sub { 0 },
    '""' => sub { $_[0] },
    fallback => 1,
);

sub TO_JSON {
    return overload::StrVal($_[0]);
}


package OverloadedObject;

use overload 'eq' => sub { $_[0]->{v} eq $_[1] }, '""' => sub { $_[0]->{v} }, fallback => 1;


sub new {
    bless { v => $_[1] }, $_[0];
}


sub TO_JSON { "$_[0]"; }

