use strict;
use warnings;

BEGIN {
	unless ($] >= 5.006) {
		print "1..0\n";
		exit(0);
	}
        if ($ENV{PERL_CORE}) {
                chdir 't' if -d 't';
                @INC = '../lib';
        }
}

use Test;
plan tests => 11;

require MIME::Base64;
require MIME::QuotedPrint;

eval {
    my $tmp = MIME::Base64::encode(v300);
    print "# enc: $tmp\n";
};
print "# $@" if $@;
ok($@);

eval {
    my $tmp = MIME::QuotedPrint::encode(v300);
    print "# enc: $tmp\n";
};
print "# $@" if $@;
ok($@);

if (defined &utf8::is_utf8) {
    my $str = "aaa" . v300;
    ok(utf8::is_utf8($str));
    chop($str);
    ok(utf8::is_utf8($str));
    ok(MIME::Base64::encode($str, ""), "YWFh");
    ok(utf8::is_utf8($str));
    ok(MIME::QuotedPrint::encode($str), "aaa=\n");
    ok(utf8::is_utf8($str));

    utf8::downgrade($str);
    ok(!utf8::is_utf8($str));
    ok(MIME::Base64::encode($str, ""), "YWFh");
    ok(!utf8::is_utf8($str));
}
else {
    skip("Missing is_utf8") for 1..9;
}
