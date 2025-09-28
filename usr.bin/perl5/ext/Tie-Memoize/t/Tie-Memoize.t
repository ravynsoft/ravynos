#!./perl -w

use strict;
use Tie::Memoize;
use Test::More tests => 27;
use File::Spec;

sub slurp {
  my ($key, $dir) = @_;
  open my $h, '<', File::Spec->catfile($dir, $key) or return;
  local $/;
  <$h>			# slurp it all
}
sub exists { my ($key, $dir) = @_; return -f File::Spec->catfile($dir, $key) }

chdir(File::Spec->updir()) if not -d 't';

my $directory = File::Spec->catdir('lib', 'Tie');

tie my %hash, 'Tie::Memoize', \&slurp, $directory, \&exists,
    { fake_file1 => 123, fake_file2 => 45678 },
    { 'strict.pm' => 0, known_to_exist => 1 };

ok(not exists $hash{'strict.pm'});
ok(exists $hash{known_to_exist});
ok($hash{fake_file2} eq 45678);
ok($hash{fake_file1} eq 123);
ok(exists $hash{known_to_exist});
ok(not exists $hash{'strict.pm'});
ok(not defined $hash{fake_file3});
ok(not defined $hash{known_to_exist});
ok(not exists $hash{known_to_exist});
ok(not exists $hash{'strict.pm'});
my $c = slurp('Memoize.pm', $directory);
ok($c);
ok($hash{'Memoize.pm'} eq $c);
ok($hash{'Memoize.pm'} eq $c);
ok(not exists $hash{'strict.pm'});
ok(exists $hash{'blib.pm'});

untie %hash;

tie %hash, 'Tie::Memoize', \&slurp, $directory;

ok(exists $hash{'Memoize.pm'}, 'existing file');
ok(not exists $hash{fake_file2});
ok(not exists $hash{fake_file1});
ok(not exists $hash{known_to_exist});
ok(exists $hash{'Memoize.pm'}, 'existing file again');
ok(not defined $hash{fake_file3});
ok(not defined $hash{known_to_exist});
ok(not exists $hash{known_to_exist});
ok(exists $hash{'Memoize.pm'}, 'existing file again');
ok($hash{'Memoize.pm'} eq $c);
ok($hash{'Memoize.pm'} eq $c);
ok(exists $hash{'Memoize.pm'}, 'existing file again');

