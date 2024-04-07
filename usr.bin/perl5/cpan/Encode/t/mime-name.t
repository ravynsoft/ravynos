#
# $Id: mime-name.t,v 1.3 2017/10/06 22:21:53 dankogai Exp $
# This script is written in utf8
#
BEGIN {
    if ($ENV{'PERL_CORE'}){
        chdir 't';
        unshift @INC, '../lib';
    }
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") == 193) {
    print "1..0 # Skip: EBCDIC\n";
    exit 0;
    }
    $| = 1;
}

use strict;
use warnings;
use Encode;
#use Test::More qw(no_plan);
use Test::More tests => 281;

BEGIN {
    use_ok("Encode::MIME::Name");
}

for my $canon ( sort keys %Encode::MIME::Name::MIME_NAME_OF ) {
    my $enc       = find_encoding($canon);
    my $mime_name = $Encode::MIME::Name::MIME_NAME_OF{$canon};
    is $enc->mime_name, $mime_name,
      qq(find_encoding($canon)->mime_name eq $mime_name);
    is $enc->name, $canon,
      qq(find_encoding($canon)->name eq $canon);
}
for my $mime_name ( sort keys %Encode::MIME::Name::ENCODE_NAME_OF ) {
    my $enc       = find_mime_encoding($mime_name);
    my $canon     = $Encode::MIME::Name::ENCODE_NAME_OF{$mime_name};
    my $mime_name = $Encode::MIME::Name::MIME_NAME_OF{$canon};
    is $enc->mime_name, $mime_name,
      qq(find_mime_encoding($mime_name)->mime_name eq $mime_name);
    is $enc->name, $canon,
      qq(find_mime_encoding($mime_name)->name eq $canon);
}

ok find_encoding("utf8");
ok find_encoding("UTF8");
ok find_encoding("utf-8-strict");
ok find_encoding("utf-8");
ok find_encoding("UTF-8");

ok not find_mime_encoding("utf8");
ok not find_mime_encoding("UTF8");
ok not find_mime_encoding("utf-8-strict");
ok find_mime_encoding("utf-8");
ok find_mime_encoding("UTF-8");

__END__;
