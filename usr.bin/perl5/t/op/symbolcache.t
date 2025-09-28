#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan( tests => 8 );

use strict;

# first, with delete
# simple removal
sub removed { 23 }
sub bound { removed() }
delete $main::{removed};
is( bound(), 23, 'function still bound' );
ok( !main->can('removed'), 'function not available as method' );

# replacement
sub replaced { 'func' }
is( replaced(), 'func', 'original function still bound' );
is( main->replaced, 'meth', 'method is replaced function' );
BEGIN { delete $main::{replaced} }
sub replaced { 'meth' }

# and now with undef
# simple removal
sub removed2 { 24 }
sub bound2 { removed2() }
{ no strict; undef *{"removed2"} }
eval { bound2() };
like( $@, qr/Undefined subroutine &main::removed2 called/,
    'function not bound' );
ok( !main->can('removed2'), 'function not available as method' );

# replacement
sub replaced2 { 'func' }
is( replaced2(), 'meth', 'original function not bound, was replaced' );
ok( main->replaced2 eq 'meth', 'method is replaced function' );
BEGIN { undef $main::{replaced2} }
sub replaced2 { 'meth' }
