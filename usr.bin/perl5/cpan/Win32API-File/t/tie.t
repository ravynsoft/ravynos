#!perl
# vim:syntax=perl:

BEGIN {
    $|= 1;

    use Test::More;

    # when building perl, skip this test if Win32API::File isn't being built
    if ( $ENV{PERL_CORE} ) {
        require Config;
        if ( $Config::Config{extensions} !~ m:(?<!\S)Win32API/File(?!\S): ) {
            plan skip_all => 'Skip Win32API::File extension not built';
            exit;
        }
    }

    plan tests => 10;
}

use strict;
use warnings;
use Win32API::File qw(:ALL);
use IO::File;

my $filename = 'foo.txt';
ok(! -e $filename || unlink($filename), "unlinked $filename (if it existed)");

my $fh = Win32API::File->new("+> $filename")
    or die fileLastError();

my $tell = tell $fh;
is(0+$tell, 0, "tell \$fh == '$tell'");

my $text = "some text\n";

ok(print($fh $text), "printed 'some text\\n'");

$tell = tell $fh;
my $len = length($text) + 1; # + 1 for cr
is($tell, $len, "after printing 'some text\\n', tell is: '$tell'");

my $seek = seek($fh, 0, 0);
is(0+$seek, 0, "seek is: '$seek'");

my $eof = eof $fh;
ok(! $eof, 'not eof');

my $readline = <$fh>;

my $pretty_readline = $readline;
$pretty_readline =~ s/\r/\\r/g;  $pretty_readline =~ s/\n/\\n/g;
is($pretty_readline, "some text\\r\\n", "read line is '$pretty_readline'");

$eof = eof $fh;
ok($eof, 'reached eof');

ok(close($fh), 'closed filehandle');

# Test out binmode (should be only LF with print, no CR).

$fh = Win32API::File->new("+> $filename")
    or die fileLastError();
binmode $fh;
print $fh "hello there\n";
seek $fh, 0, 0;

$readline = <$fh>;
is($readline, "hello there\n", "binmode worked (no CR)");

close $fh;

unlink $filename;
