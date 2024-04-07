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

  is_deeply(
    $req->as_string_hash,
    {
      'Foo::Bar'   => 10,
      'Foo::Baz'   => 'v1.2.3',
      'Foo::Undef' => 0,
    },
    "some basic minimums",
  );

  ok($req->is_simple, "just minimums? simple");
}

{
  my $req = CPAN::Meta::Requirements->new;
  $req->add_maximum(Foo => 1);
  is_deeply($req->as_string_hash, { Foo => '<= 1' }, "max only");

  ok(! $req->is_simple, "maximums? not simple");
}

{
  my $req = CPAN::Meta::Requirements->new;
  $req->add_exclusion(Foo => 1);
  $req->add_exclusion(Foo => 2);

  # Why would you ever do this?? -- rjbs, 2010-02-20
  is_deeply($req->as_string_hash, { Foo => '!= 1, != 2' }, "excl only");
}

{
  my $req = CPAN::Meta::Requirements->new;

  $req->add_minimum(Foo => 1);
  $req->add_maximum(Foo => 2);

  is_deeply(
    $req->as_string_hash,
    {
      Foo => '>= 1, <= 2',
    },
    "min and max",
  );

  $req->add_maximum(Foo => 3);

  is_deeply(
    $req->as_string_hash,
    {
      Foo => '>= 1, <= 2',
    },
    "exclusions already outside range do not matter",
  );

  $req->add_exclusion(Foo => 1.5);

  is_deeply(
    $req->as_string_hash,
    {
      Foo => '>= 1, <= 2, != 1.5',
    },
    "exclusions",
  );

  $req->add_minimum(Foo => 1.6);

  is_deeply(
    $req->as_string_hash,
    {
      Foo => '>= 1.6, <= 2',
    },
    "exclusions go away when made irrelevant",
  );
}

{
  my $req = CPAN::Meta::Requirements->new;

  $req->add_minimum(Foo => 1);
  $req->add_exclusion(Foo => 1);
  $req->add_maximum(Foo => 2);

  is_deeply(
    $req->as_string_hash,
    {
      Foo => '> 1, <= 2',
    },
    "we can exclude an endpoint",
  );
}

{
  my $req = CPAN::Meta::Requirements->new;
  $req->add_minimum(Foo => 1);

  $req->add_exclusion(Foo => 1);

  dies_ok { $req->add_maximum(Foo => 1); }
    qr/both 1, which is excluded/,
    "can't exclude all values" ;
}

{
  my $req = CPAN::Meta::Requirements->new;
  $req->add_minimum(Foo => 1);
  dies_ok {$req->exact_version(Foo => 0.5); }
    qr/outside of range/,
    "can't add outside-range exact spec to range";
}

{
  my $req = CPAN::Meta::Requirements->new;
  $req->add_minimum(Foo => 1);
  dies_ok { $req->add_maximum(Foo => 0.5); }
    qr/minimum 1 exceeds maximum/,
    "maximum must exceed (or equal) minimum";

  $req = CPAN::Meta::Requirements->new;
  $req->add_maximum(Foo => 0.5);
  dies_ok { $req->add_minimum(Foo => 1); }
    qr/minimum 1 exceeds maximum/,
    "maximum must exceed (or equal) minimum";
}

{
  my $req = CPAN::Meta::Requirements->new;

  $req->add_minimum(Foo => 1);
  $req->add_maximum(Foo => 1);

  $req->add_maximum(Foo => 2); # ignored
  $req->add_minimum(Foo => 0); # ignored
  $req->add_exclusion(Foo => .5); # ignored

  is_deeply(
    $req->as_string_hash,
    {
      'Foo' => '== 1',
    },
    "if min==max, becomes exact requirement",
  );
}

{
  my $req = CPAN::Meta::Requirements->new;
  $req->add_minimum(Foo => 1);
  $req->add_exclusion(Foo => 0);
  $req->add_maximum(Foo => 3);
  $req->add_exclusion(Foo => 4);

  $req->add_exclusion(Foo => 2);
  $req->add_exclusion(Foo => 2);

  is_deeply(
    $req->as_string_hash,
    {
      Foo => '>= 1, <= 3, != 2',
    },
    'test exclusion-skipping',
  );

  is_deeply(
    $req->structured_requirements_for_module('Foo'),
    # remember, it's okay to change the exact results, as long as the meaning
    # is unchanged -- rjbs, 2012-07-11
    [
      [ '>=', '1' ],
      [ '<=', '3' ],
      [ '!=', '2' ],
    ],
    "structured requirements for Foo",
  );
}

sub foo_1 {
  my $req = CPAN::Meta::Requirements->new;
  $req->exact_version(Foo => 1);
  return $req;
}

{
  my $req = foo_1;

  $req->exact_version(Foo => 1); # ignored

  is_deeply($req->as_string_hash, { Foo => '== 1' }, "exact requirement");

  dies_ok { $req->exact_version(Foo => 2); }
    qr/can't be exactly 2.+already/,
    "can't exactly specify differing versions" ;

  $req = foo_1;
  $req->add_minimum(Foo => 0); # ignored
  $req->add_maximum(Foo => 2); # ignored

  dies_ok { $req->add_maximum(Foo => 0); } qr/maximum 0 below exact/, "max < fixed";

  $req = foo_1;
  dies_ok { $req->add_minimum(Foo => 2); } qr/minimum 2 exceeds exact/, "min > fixed";

  $req = foo_1;
  $req->add_exclusion(Foo => 8); # ignored
  dies_ok { $req->add_exclusion(Foo => 1); } qr/tried to exclude/, "!= && ==";
}

{
  my $req = foo_1;

  is($req->requirements_for_module('Foo'), '== 1', 'requirements_for_module');

  is_deeply(
    $req->structured_requirements_for_module('Foo'),
    [ [ '==', '1' ] ],
    'structured_requirements_for_module'
  );

  # test empty/undef returns
  my @list = $req->requirements_for_module('FooBarBamBaz');
  my $scalar = $req->requirements_for_module('FooBarBamBaz');
  is ( scalar @list, 0, "requirements_for_module() returns empty for not found (list)" );
  is ( $scalar, undef, "requirements_for_module() returns undef for not found (scalar)" );
}

{
  my $req = CPAN::Meta::Requirements->new;

  $req->add_minimum(Foo => "0.00");

  my $req2 = CPAN::Meta::Requirements->new;
  $req2->add_requirements($req);

  is_deeply(
    $req2->as_string_hash,
    {
      Foo => '0.00'
    },
    "0.00 precision preserved",
  );

}

done_testing;
