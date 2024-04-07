use Digest::MD5;
use Test::More tests => 6;

$^W = 0; # No warnings
{
  local *STDERR;
  my $stderr_seen = "";
  open STDERR, '>', \$stderr_seen;
  $str = Digest::MD5->md5_hex("foo");
  is($stderr_seen,'','No warnings');
}

{
  $^W = 1; # magic turn on warnings
  local *STDERR;
  my $stderr_seen = "";
  open STDERR, '>', \$stderr_seen;
  $str = Digest::MD5->md5_hex("foo");
  like($stderr_seen,qr/Digest::MD5::md5_hex function probably called as class method/,
        'Lexical warning passed to XSUB');
}

{
  $^W = 0; # No warnings
  local *STDERR;
  my $stderr_seen = "";
  open STDERR, '>', \$stderr_seen;
  $str = Digest::MD5->md5_hex("foo");
  is($stderr_seen,'','No warnings again');
}

{
  use warnings;
  local *STDERR;
  my $stderr_seen = "";
  open STDERR, '>', \$stderr_seen;
  $str = Digest::MD5->md5_hex("foo");
  like($stderr_seen,qr/Digest::MD5::md5_hex function probably called as class method/,
        'use warnings passed to XSUB');
}

{
  use strict;
  $^W = 0; # No warnings
  local *STDERR;
  my $stderr_seen = "";
  open STDERR, '>', \$stderr_seen;
  my $str = Digest::MD5->md5_hex("foo");
  is($stderr_seen,'','No warnings and strict');
}

{
  use strict;
  use warnings;
  local *STDERR;
  my $stderr_seen = "";
  open STDERR, '>', \$stderr_seen;
  my $str = Digest::MD5->md5_hex("foo");
  like($stderr_seen, qr/Digest::MD5::md5_hex function probably called as class method/,
        'use warnings passed to XSUB while use strict');
}

