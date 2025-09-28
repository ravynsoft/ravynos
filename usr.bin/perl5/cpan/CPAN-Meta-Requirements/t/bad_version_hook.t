use strict;
use warnings;

use CPAN::Meta::Requirements;
use version;

use Test::More 0.88;

my %DATA = (
  'Foo::Bar' => [ 10, 10 ],
  'Foo::Baz' => [ 'invalid_version', 42 ],
  'Foo::Qux' => [ 'version', 42 ],
);
my %input = map { ($_ => $DATA{$_}->[0]) } keys %DATA;
my %expected = map { ($_ => $DATA{$_}->[1]) } keys %DATA;

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

my $hook_text;
sub _fixit { my ($v, $m) = @_; $hook_text .= $m; return version->new(42) }

{
  my $req = CPAN::Meta::Requirements->new( {bad_version_hook => \&_fixit} );

  my ($k, $v);
  while (($k, $v) = each %input) {
    note "adding minimum requirement: $k => $v";
    eval { $req->add_minimum($k => $v) };
    is( $@, '', "adding minimum '$k' for $v" );
  }
  like( $hook_text, qr/Foo::Baz/, 'hook stored module name' );

  is_deeply(
    $req->as_string_hash,
    \%expected,
    "hook fixes invalid version",
  );
}

{
  my $req = CPAN::Meta::Requirements->new( {bad_version_hook => sub { 0 }} );

  dies_ok { $req->add_minimum('Foo::Baz' => 'invalid_version') }
    qr/Invalid version/,
    "dies if hook doesn't return version object";

}


done_testing;
