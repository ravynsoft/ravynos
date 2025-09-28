use strict;
use warnings;
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

use CPAN::Meta::Requirements;

my $req = CPAN::Meta::Requirements->new;

# Test ==
$req->add_string_requirement('Foo::Bar', '== 1.3');
ok($req->accepts_module('Foo::Bar' => '1.3'), 'exact version (==)');
ok(!$req->accepts_module('Foo::Bar' => '1.2'), 'lower version (==)');
ok(!$req->accepts_module('Foo::Bar' => '1.4'), 'higher version (==)');

# Test !=
$req->add_string_requirement('Foo::Baz', '!= 1.3');
ok(!$req->accepts_module('Foo::Baz' => '1.3'), 'exact version (!=)');
ok($req->accepts_module('Foo::Baz' => '1.2'), 'lower version (!=)');
ok($req->accepts_module('Foo::Baz' => '1.4'), 'higher version (!=)');

# Test >=
$req->add_string_requirement('Foo::Gorch', '>= 1.3');
ok($req->accepts_module('Foo::Gorch' => '1.3'), 'exact version (>=)');
ok(!$req->accepts_module('Foo::Gorch' => '1.2'), 'lower version (>=)');
ok($req->accepts_module('Foo::Gorch' => '1.4'), 'higher version (>=)');

# Test <=
$req->add_string_requirement('Foo::Graz', '<= 1.3');
ok($req->accepts_module('Foo::Graz' => '1.3'), 'exact version (<=)');
ok($req->accepts_module('Foo::Graz' => '1.2'), 'lower version (<=)');
ok(!$req->accepts_module('Foo::Graz' => '1.4'), 'higher version (<=)');

# Test ""
$req->add_string_requirement('Foo::Blurb', '>= 1.3');
ok($req->accepts_module('Foo::Blurb' => '1.3'), 'exact version (>=)');
ok(!$req->accepts_module('Foo::Blurb' => '1.2'), 'lower version (>=)');
ok($req->accepts_module('Foo::Blurb' => '1.4'), 'higher version (>=)');

# Test multiple requirements
$req->add_string_requirement('A::Tribe::Called', '>= 1.3, <= 2.0, != 1.6');
ok($req->accepts_module('A::Tribe::Called' => '1.5'), 'middle version (>=, <=, !)');
ok(!$req->accepts_module('A::Tribe::Called' => '1.2'), 'lower version (>=, <=, !)');
ok(!$req->accepts_module('A::Tribe::Called' => '2.1'), 'higher version (>=, <=, !)');
ok(!$req->accepts_module('A::Tribe::Called' => '1.6'), 'excluded version (>=, <=, !)');

# Test precision
{
  my $req = CPAN::Meta::Requirements->new;

  $req->add_string_requirement(Foo => "0.00");

  is_deeply(
    $req->as_string_hash,
    {
      Foo => '0.00'
    },
    "0.00 precision preserved",
  );
}

# Test fatal errors
dies_ok { $req->add_string_requirement('Foo::Bar', "not really a version") }
  qr/Can't convert/,
  "conversion failure caught";

done_testing;
