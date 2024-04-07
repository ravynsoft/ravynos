#!perl -w

use strict;
use warnings;
use Test::More tests => 2;

use XS::APItest;

is join("", sort xs_cmp split//, '1415926535'), '1135559246',
  'sort treats XS cmp routines as having implicit ($$)';
{
  my $w;
  local $SIG{__WARN__} = sub { $w .= shift };
  () = sort xs_cmp_undef 1,2;
  like $w, qr/^Use of uninitialized value in sort at /,
   'warning about undef retval from cmp routine mentions sort';
}
