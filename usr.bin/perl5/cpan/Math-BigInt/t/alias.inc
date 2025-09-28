# -*- mode: perl; -*-

use strict;
use warnings;

our $CLASS;

# alias subroutine testing, included by sub_ali.t, mbi_ali.t, and mbf_ali.t

our $x = $CLASS->new(123);

is($x->is_pos(), 1, "$CLASS -> new(123) -> is_pos()");
is($x->is_neg(), 0, "$CLASS -> new(123) -> is_neg()");
is($x->as_int(), 123, "$CLASS -> new(123) -> as_int()");
is(ref($x->as_int()), 'Math::BigInt', "ref($CLASS -> new(123) -> as_int())");
$x->bneg();
is($x->is_pos(), 0, "$CLASS -> new(123) -> bneg() -> is_pos()");
is($x->is_neg(), 1, "$CLASS -> new(123) -> bneg() -> is_neg()");
