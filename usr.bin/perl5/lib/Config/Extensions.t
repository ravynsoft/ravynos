#!perl -w
BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't' if -d 't';
        @INC = '../lib';
    }
}
use strict;
use Test::More;

BEGIN {use_ok 'Config::Extensions', '%Extensions'};

use Config;

my @types = qw(dynamic static nonxs);
my %types;
@types{@types} = @types;

ok (keys %Extensions, "There are some extensions");
# Check only the 3 valid keys have been used.
while (my ($key, $val) = each %Extensions) {
    my $raw_ext = $key;
    # Back to the format in Config
    $raw_ext =~ s!::!/!g;
    my $re = qr/\b\Q$raw_ext\E\b/;
    like($Config{extensions}, $re, "$key was built");
    unless ($types{$val}) {
	fail("$key is $val");
	next;
    }
    my $type = $val . '_ext';
    like($Config{$type}, $re, "$key is $type");
}

done_testing();
