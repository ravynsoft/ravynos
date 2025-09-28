use strict;
use warnings;

BEGIN {
    if ($ENV{'PERL_CORE'}){
        chdir 't' if -d 't';
        @INC = '../lib';
    }
}

use MIME::Base64;

print "1..283\n";

print "# Testing MIME::Base64-", $MIME::Base64::VERSION, "\n";

BEGIN {
 if (ord('A') == 0x41) {
  *ASCII = sub { return $_[0] };
 }
 else {
  require Encode;
  *ASCII = sub { Encode::encode('ascii',$_[0]) };
 }
}

my $testno = 1;
# instead of "for my $test (...)" , which is my preference.
# Not sure which perl version has started supporting.  MIME::Base64
# was supposed to work with very old perl5, right?
my $test;

encodeTest();
decodeTest();

# This used to generate a warning
print "not " unless decode_base64(encode_base64("foo")) eq "foo";
print "ok ", $testno++, "\n";

sub encodeTest
{
    print "# encode test\n";

    my @encode_tests = (
	# All values
	["\000" => "AA=="],
	["\001" => "AQ=="],
	["\002" => "Ag=="],
	["\003" => "Aw=="],
	["\004" => "BA=="],
	["\005" => "BQ=="],
	["\006" => "Bg=="],
	["\007" => "Bw=="],
	["\010" => "CA=="],
	["\011" => "CQ=="],
	["\012" => "Cg=="],
	["\013" => "Cw=="],
	["\014" => "DA=="],
	["\015" => "DQ=="],
	["\016" => "Dg=="],
	["\017" => "Dw=="],
	["\020" => "EA=="],
	["\021" => "EQ=="],
	["\022" => "Eg=="],
	["\023" => "Ew=="],
	["\024" => "FA=="],
	["\025" => "FQ=="],
	["\026" => "Fg=="],
	["\027" => "Fw=="],
	["\030" => "GA=="],
	["\031" => "GQ=="],
	["\032" => "Gg=="],
	["\033" => "Gw=="],
	["\034" => "HA=="],
	["\035" => "HQ=="],
	["\036" => "Hg=="],
	["\037" => "Hw=="],
	["\040" => "IA=="],
	["\041" => "IQ=="],
	["\042" => "Ig=="],
	["\043" => "Iw=="],
	["\044" => "JA=="],
	["\045" => "JQ=="],
	["\046" => "Jg=="],
	["\047" => "Jw=="],
	["\050" => "KA=="],
	["\051" => "KQ=="],
	["\052" => "Kg=="],
	["\053" => "Kw=="],
	["\054" => "LA=="],
	["\055" => "LQ=="],
	["\056" => "Lg=="],
	["\057" => "Lw=="],
	["\060" => "MA=="],
	["\061" => "MQ=="],
	["\062" => "Mg=="],
	["\063" => "Mw=="],
	["\064" => "NA=="],
	["\065" => "NQ=="],
	["\066" => "Ng=="],
	["\067" => "Nw=="],
	["\070" => "OA=="],
	["\071" => "OQ=="],
	["\072" => "Og=="],
	["\073" => "Ow=="],
	["\074" => "PA=="],
	["\075" => "PQ=="],
	["\076" => "Pg=="],
	["\077" => "Pw=="],
	["\100" => "QA=="],
	["\101" => "QQ=="],
	["\102" => "Qg=="],
	["\103" => "Qw=="],
	["\104" => "RA=="],
	["\105" => "RQ=="],
	["\106" => "Rg=="],
	["\107" => "Rw=="],
	["\110" => "SA=="],
	["\111" => "SQ=="],
	["\112" => "Sg=="],
	["\113" => "Sw=="],
	["\114" => "TA=="],
	["\115" => "TQ=="],
	["\116" => "Tg=="],
	["\117" => "Tw=="],
	["\120" => "UA=="],
	["\121" => "UQ=="],
	["\122" => "Ug=="],
	["\123" => "Uw=="],
	["\124" => "VA=="],
	["\125" => "VQ=="],
	["\126" => "Vg=="],
	["\127" => "Vw=="],
	["\130" => "WA=="],
	["\131" => "WQ=="],
	["\132" => "Wg=="],
	["\133" => "Ww=="],
	["\134" => "XA=="],
	["\135" => "XQ=="],
	["\136" => "Xg=="],
	["\137" => "Xw=="],
	["\140" => "YA=="],
	["\141" => "YQ=="],
	["\142" => "Yg=="],
	["\143" => "Yw=="],
	["\144" => "ZA=="],
	["\145" => "ZQ=="],
	["\146" => "Zg=="],
	["\147" => "Zw=="],
	["\150" => "aA=="],
	["\151" => "aQ=="],
	["\152" => "ag=="],
	["\153" => "aw=="],
	["\154" => "bA=="],
	["\155" => "bQ=="],
	["\156" => "bg=="],
	["\157" => "bw=="],
	["\160" => "cA=="],
	["\161" => "cQ=="],
	["\162" => "cg=="],
	["\163" => "cw=="],
	["\164" => "dA=="],
	["\165" => "dQ=="],
	["\166" => "dg=="],
	["\167" => "dw=="],
	["\170" => "eA=="],
	["\171" => "eQ=="],
	["\172" => "eg=="],
	["\173" => "ew=="],
	["\174" => "fA=="],
	["\175" => "fQ=="],
	["\176" => "fg=="],
	["\177" => "fw=="],
	["\200" => "gA=="],
	["\201" => "gQ=="],
	["\202" => "gg=="],
	["\203" => "gw=="],
	["\204" => "hA=="],
	["\205" => "hQ=="],
	["\206" => "hg=="],
	["\207" => "hw=="],
	["\210" => "iA=="],
	["\211" => "iQ=="],
	["\212" => "ig=="],
	["\213" => "iw=="],
	["\214" => "jA=="],
	["\215" => "jQ=="],
	["\216" => "jg=="],
	["\217" => "jw=="],
	["\220" => "kA=="],
	["\221" => "kQ=="],
	["\222" => "kg=="],
	["\223" => "kw=="],
	["\224" => "lA=="],
	["\225" => "lQ=="],
	["\226" => "lg=="],
	["\227" => "lw=="],
	["\230" => "mA=="],
	["\231" => "mQ=="],
	["\232" => "mg=="],
	["\233" => "mw=="],
	["\234" => "nA=="],
	["\235" => "nQ=="],
	["\236" => "ng=="],
	["\237" => "nw=="],
	["\240" => "oA=="],
	["\241" => "oQ=="],
	["\242" => "og=="],
	["\243" => "ow=="],
	["\244" => "pA=="],
	["\245" => "pQ=="],
	["\246" => "pg=="],
	["\247" => "pw=="],
	["\250" => "qA=="],
	["\251" => "qQ=="],
	["\252" => "qg=="],
	["\253" => "qw=="],
	["\254" => "rA=="],
	["\255" => "rQ=="],
	["\256" => "rg=="],
	["\257" => "rw=="],
	["\260" => "sA=="],
	["\261" => "sQ=="],
	["\262" => "sg=="],
	["\263" => "sw=="],
	["\264" => "tA=="],
	["\265" => "tQ=="],
	["\266" => "tg=="],
	["\267" => "tw=="],
	["\270" => "uA=="],
	["\271" => "uQ=="],
	["\272" => "ug=="],
	["\273" => "uw=="],
	["\274" => "vA=="],
	["\275" => "vQ=="],
	["\276" => "vg=="],
	["\277" => "vw=="],
	["\300" => "wA=="],
	["\301" => "wQ=="],
	["\302" => "wg=="],
	["\303" => "ww=="],
	["\304" => "xA=="],
	["\305" => "xQ=="],
	["\306" => "xg=="],
	["\307" => "xw=="],
	["\310" => "yA=="],
	["\311" => "yQ=="],
	["\312" => "yg=="],
	["\313" => "yw=="],
	["\314" => "zA=="],
	["\315" => "zQ=="],
	["\316" => "zg=="],
	["\317" => "zw=="],
	["\320" => "0A=="],
	["\321" => "0Q=="],
	["\322" => "0g=="],
	["\323" => "0w=="],
	["\324" => "1A=="],
	["\325" => "1Q=="],
	["\326" => "1g=="],
	["\327" => "1w=="],
	["\330" => "2A=="],
	["\331" => "2Q=="],
	["\332" => "2g=="],
	["\333" => "2w=="],
	["\334" => "3A=="],
	["\335" => "3Q=="],
	["\336" => "3g=="],
	["\337" => "3w=="],
	["\340" => "4A=="],
	["\341" => "4Q=="],
	["\342" => "4g=="],
	["\343" => "4w=="],
	["\344" => "5A=="],
	["\345" => "5Q=="],
	["\346" => "5g=="],
	["\347" => "5w=="],
	["\350" => "6A=="],
	["\351" => "6Q=="],
	["\352" => "6g=="],
	["\353" => "6w=="],
	["\354" => "7A=="],
	["\355" => "7Q=="],
	["\356" => "7g=="],
	["\357" => "7w=="],
	["\360" => "8A=="],
	["\361" => "8Q=="],
	["\362" => "8g=="],
	["\363" => "8w=="],
	["\364" => "9A=="],
	["\365" => "9Q=="],
	["\366" => "9g=="],
	["\367" => "9w=="],
	["\370" => "+A=="],
	["\371" => "+Q=="],
	["\372" => "+g=="],
	["\373" => "+w=="],
	["\374" => "/A=="],
	["\375" => "/Q=="],
	["\376" => "/g=="],
	["\377" => "/w=="],

	["\000\377" => "AP8="],
	["\377\000" => "/wA="],
	["\000\000\000" => "AAAA"],

        [''    => ''],
	[ASCII('a')   => 'YQ=='],
	[ASCII('aa')  => 'YWE='],
	[ASCII('aaa') => 'YWFh'],

	[ASCII('aaa') => 'YWFh'],
	[ASCII('aaa') => 'YWFh'],
	[ASCII('aaa') => 'YWFh'],


	# from HTTP spec
	[ASCII('Aladdin:open sesame') => 'QWxhZGRpbjpvcGVuIHNlc2FtZQ=='],

	[ASCII('a') x 100 => 'YWFh' x 33 . 'YQ=='],

	[ASCII('Multipurpose Internet Mail Extensions: The Base64 Content-Transfer-Encoding is designed to represent sequences of octets in a form that is not humanly readable. ')
	=> "TXVsdGlwdXJwb3NlIEludGVybmV0IE1haWwgRXh0ZW5zaW9uczogVGhlIEJhc2U2NCBDb250ZW50LVRyYW5zZmVyLUVuY29kaW5nIGlzIGRlc2lnbmVkIHRvIHJlcHJlc2VudCBzZXF1ZW5jZXMgb2Ygb2N0ZXRzIGluIGEgZm9ybSB0aGF0IGlzIG5vdCBodW1hbmx5IHJlYWRhYmxlLiA="],

    );

    for $test (@encode_tests) {
	my($plain, $expected) = ($$test[0], $$test[1]);

	my $encoded = encode_base64($plain, '');
	if ($encoded ne $expected) {
	    print "test $testno ($plain): expected $expected, got $encoded\n";
            print "not ";
	}
	my $decoded = decode_base64($encoded);
	if ($decoded ne $plain) {
	    print "test $testno ($encoded): expected $plain, got $decoded\n";
            print "not ";
	}

	print "ok $testno\n";
	$testno++;
    }
}

sub decodeTest
{
    print "# decode test\n";

    local $SIG{__WARN__} = sub { print $_[0] };  # avoid warnings on stderr

    my @decode_tests = (
	['YWE='   => ASCII('aa')],
	[' YWE='  =>  ASCII('aa')],
	['Y WE='  =>  ASCII('aa')],
	['YWE= '  =>  ASCII('aa')],
	["Y\nW\r\nE=" =>  ASCII('aa')],

	# These will generate some warnings
        ['YWE=====' =>  ASCII('aa')],    # extra padding
	['YWE'      =>  ASCII('aa')],    # missing padding
        ['YWFh====' =>  ASCII('aaa')],
        ['YQ'       =>  ASCII('a')],
        ['Y'        => ''],
        ['x=='      => ''],
        [''         => ''],
        [undef()    => ''],
    );

    for $test (@decode_tests) {
	my($encoded, $expected) = ($$test[0], $$test[1]);

	my $decoded = decode_base64($encoded);
	if ($decoded ne $expected) {
	    die "test $testno ($encoded): expected $expected, got $decoded\n";
	}
	print "ok $testno\n";
	$testno++;
    }
}
