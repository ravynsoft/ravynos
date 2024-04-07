use strict;
use warnings;

use CPAN::Meta::Requirements;

use Test::More 0.88;

sub dies_ok (&@) {
  my ($code, $qr, $comment) = @_;

  no warnings 'redefine';
  local *Regexp::CARP_TRACE  = sub { "<regexp>" };
  my $lived = eval { $code->(); 1 };

  if ($lived) {
    fail("$comment: did not die");
  } else {
    like($@, $qr, $comment);
  }
}

{
  my $req = CPAN::Meta::Requirements->new;

  $req->add_minimum('Foo::Bar' => 10);
  $req->add_minimum('Foo::Bar' => 0);
  $req->add_minimum('Foo::Bar' => 2);

  $req->add_minimum('Foo::Baz' => version->declare('v1.2.3'));

  $req->add_minimum('Foo::Undef' => undef);

  my $want = {
    'Foo::Bar'   => 10,
    'Foo::Baz'   => 'v1.2.3',
    'Foo::Undef' => 0,
  };

  is_deeply(
    $req->as_string_hash,
    $want,
    "some basic minimums",
  );

  $req->finalize;

  $req->add_minimum('Foo::Bar', 2);

  pass('we can add a Foo::Bar requirement with no effect post finalization');

  dies_ok { $req->add_minimum('Foo::Bar', 12) }
    qr{finalized req},
    "can't add a higher Foo::Bar after finalization";

  dies_ok { $req->add_minimum('Foo::New', 0) }
    qr{finalized req},
    "can't add a new module prereq after finalization";

  dies_ok { $req->clear_requirement('Foo::Bar') }
    qr{finalized req},
    "can't clear an existing prereq after finalization";

  $req->clear_requirement('Bogus::Req');

  pass('we can clear a prereq that was not set to begin with');

  is_deeply(
    $req->as_string_hash,
    $want,
    "none of our attempts to alter the object post-finalization worked",
  );

  my $cloned = $req->clone;

  $cloned->add_minimum('Foo::Bar', 12);

  is_deeply(
    $cloned->as_string_hash,
    {
      %$want,
      'Foo::Bar' => 12,
    },
    "we can alter a cloned V:R (finalization does not survive cloning)",
  );

  is_deeply(
    $req->as_string_hash,
    $want,
    "...and original requirements are untouched",
  );
}

done_testing;
