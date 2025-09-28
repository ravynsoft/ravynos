#!./perl

use File::Spec;
use lib File::Spec->catdir('t', 'lib');
use Test::More;

if( $^O eq 'MSWin32' ) {
  plan tests => 4;
} else {
  plan skip_all => 'this is not win32';
}

use Cwd;
ok 1;

my $cdir = getdcwd('C:');
like $cdir, qr{^C:}i;

my $ddir = getdcwd('D:');
if (defined $ddir) {
  like $ddir, qr{^D:}i;
} else {
  # May not have a D: drive mounted
  ok 1;
}

# Ensure compatibility with naughty versions of Template::Toolkit,
# which pass in a bare $1 as an argument
'Foo/strawberry' =~ /(.*)/;
my $result = File::Spec::Win32->catfile('C:/cache', $1);
is( $result, 'C:\cache\Foo\strawberry' );

