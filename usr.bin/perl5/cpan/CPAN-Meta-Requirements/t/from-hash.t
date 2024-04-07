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
  my $string_hash = {
    Left   => 10,
    Shared => '>= 2, <= 9, != 7',
    Right  => 18,
  };

  my $req = CPAN::Meta::Requirements->from_string_hash($string_hash);

  is_deeply(
    $req->as_string_hash,
    $string_hash,
    "we can load from a string hash",
  );
}

SKIP: {
  skip "Can't tell v-strings from strings until 5.8.1", 1
    unless $] gt '5.008';
  my $string_hash = {
    Left   => 10,
    Shared => '= 2',
    Right  => 18,
  };

  dies_ok { CPAN::Meta::Requirements->from_string_hash($string_hash) }
    qr/Can't convert/,
    "we die when we can't understand a version spec";
}

{
  my $undef_hash = { Undef => undef };
  my $z_hash = { ZeroLength => '' };

  my $warning;
  local $SIG{__WARN__} = sub { $warning = join("\n",@_) };

  my $req = CPAN::Meta::Requirements->from_string_hash($undef_hash);
  like ($warning, qr/Undefined requirement.*treated as '0'/, "undef requirement warns");
  $req->add_string_requirement(%$z_hash);
  like ($warning, qr/Undefined requirement.*treated as '0'/, "'' requirement warns");

  is_deeply(
    $req->as_string_hash,
    { map { ($_ => 0) } keys(%$undef_hash), keys(%$z_hash) },
    "undef/'' requirements treated as '0'",
  );
}

SKIP: {
  skip "Can't tell v-strings from strings until 5.8.1", 2
    unless $] gt '5.008';
  my $string_hash = {
    Left   => 10,
    Shared => v50.44.60,
    Right  => 18,
  };

  my $warning;
  local $SIG{__WARN__} = sub { $warning = join("\n",@_) };

  my $req = eval { CPAN::Meta::Requirements->from_string_hash($string_hash); };
  is( $@, '', "vstring in string hash lives" );

  ok(
    $req->accepts_module(Shared => 'v50.44.60'),
    "vstring treated as if string",
  );
}


{
  my $req = CPAN::Meta::Requirements->from_string_hash(
    { Bad => 'invalid', },
    { bad_version_hook => sub { version->new(42) } },
  );

  ok(
    $req->accepts_module(Bad => 42),
    "options work 2nd arg to f_s_h",
  );
}

done_testing;
