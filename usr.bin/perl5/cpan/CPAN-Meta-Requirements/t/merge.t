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
  my $req_1 = CPAN::Meta::Requirements->new;
  $req_1->add_minimum(Left   => 10);
  $req_1->add_minimum(Shared => 2);
  $req_1->add_exclusion(Shared => 7);

  my $req_2 = CPAN::Meta::Requirements->new;
  $req_2->add_minimum(Shared => 1);
  $req_2->add_maximum(Shared => 9);
  $req_2->add_minimum(Right  => 18);

  $req_1->add_requirements($req_2);

  is_deeply(
    $req_1->as_string_hash,
    {
      Left   => 10,
      Shared => '>= 2, <= 9, != 7',
      Right  => 18,
    },
    "add requirements to an existing set of requirements",
  );
}

{
  my $req_1 = CPAN::Meta::Requirements->new;
  $req_1->add_minimum(Left   => 10);
  $req_1->add_minimum(Shared => 2);
  $req_1->add_exclusion(Shared => 7);
  $req_1->exact_version(Exact => 8);

  my $req_2 = CPAN::Meta::Requirements->new;
  $req_2->add_minimum(Shared => 1);
  $req_2->add_maximum(Shared => 9);
  $req_2->add_minimum(Right  => 18);
  $req_2->exact_version(Exact => 8);

  my $clone = $req_1->clone->add_requirements($req_2);

  is_deeply(
    $req_1->as_string_hash,
    {
      Left   => 10,
      Shared => '>= 2, != 7',
      Exact  => '== 8',
    },
    "clone/add_requirements does not affect lhs",
  );

  is_deeply(
    $req_2->as_string_hash,
    {
      Shared => '>= 1, <= 9',
      Right  => 18,
      Exact  => '== 8',
    },
    "clone/add_requirements does not affect rhs",
  );

  is_deeply(
    $clone->as_string_hash,
    {
      Left   => 10,
      Shared => '>= 2, <= 9, != 7',
      Right  => 18,
      Exact  => '== 8',
    },
    "clone and add_requirements",
  );

  $clone->clear_requirement('Shared');

  is_deeply(
    $clone->as_string_hash,
    {
      Left   => 10,
      Right  => 18,
      Exact  => '== 8',
    },
    "cleared the shared requirement",
  );
}

{
  my $req_1 = CPAN::Meta::Requirements->new;
  $req_1->add_maximum(Foo => 1);

  my $req_2 = $req_1->clone;

  is_deeply(
    $req_2->as_string_hash,
    {
      'Foo' => '<= 1',
    },
    'clone with only max',
  );
}

{
  my $left = CPAN::Meta::Requirements->new;
  $left->add_minimum(Foo => 0);
  $left->add_minimum(Bar => 1);

  my $right = CPAN::Meta::Requirements->new;
  $right->add_requirements($left);

  is_deeply(
    $right->as_string_hash,
    {
      Foo => 0,
      Bar => 1,
    },
    "we do not lose 0-min reqs on merge",
  );
}

done_testing;
