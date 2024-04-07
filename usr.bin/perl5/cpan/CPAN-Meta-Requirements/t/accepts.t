use strict;
use warnings;

use CPAN::Meta::Requirements;

use Test::More 0.88;

{
  my $req = CPAN::Meta::Requirements->new->add_minimum(Foo => 1);

  ok(  $req->accepts_module(Foo => 1), "need 1, got 1");
  ok(! $req->accepts_module(Foo => 0), "need 0, got 1");
}

{
  my $req = CPAN::Meta::Requirements->new->add_minimum(Foo => 0);

  ok(  $req->accepts_module(Foo => 1), "need 0, got 1");
  ok(  $req->accepts_module(Foo => undef), "need 0, got undef");
  ok(  $req->accepts_module(Foo => "v0"), "need 0, got 'v0'");
  ok(  $req->accepts_module(Foo => v1.2.3), "need 0, got v1.2.3");
  ok(  $req->accepts_module(Foo => "v1.2.3"), "need 0, got 'v1.2.3'");
}

{
  my $req = CPAN::Meta::Requirements->new->add_maximum(Foo => 1);

  ok(  $req->accepts_module(Foo => 1), "need <=1, got 1");
  ok(! $req->accepts_module(Foo => 2), "need <=1, got 2");
}

{
  my $req = CPAN::Meta::Requirements->new->add_exclusion(Foo => 1);

  ok(  $req->accepts_module(Foo => 0), "need !1, got 0");
  ok(! $req->accepts_module(Foo => 1), "need !1, got 1");
}

done_testing;
