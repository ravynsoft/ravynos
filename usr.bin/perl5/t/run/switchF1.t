#!perl -w

# This test file does not use test.pl because of the involved way in which it
# generates its TAP output.

print "1..5\n";

my $file = "Run_switchF1.pl";

open F, ">$file" or die "Open $file: $!";

my $prog = <<'EOT';
#!./perl -anF[~#QQ\\xq']

BEGIN {
    *ARGV = *DATA;
}
print "@F";

__DATA__
okx1x- use of alternate delimiter (lower case letter) in -F
okq2q- use of alternate delimiter (lower case letter) in -F
ok\3\- use of alternate delimiter (backslash) in -F
ok'4'- use of alternate delimiter (apostrophe) in -F
EOT

# 2 of the characters toke.c used to use to quote the split parameter:
$prog =~ s/QQ/\x01\x80/;
# These 2 plus ~ # and ' were enough to make perl choke
print F $prog;
close F or die "Close $file: $!";

$count = 5;
$result = "ok $count - complete test of alternate delimiters in -F\n";
print system ($^X, $file) ? "not $result" : $result;

unlink $file or die "Unlink $file: $!";
