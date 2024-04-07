#!./perl

BEGIN {
	chdir 't' if -d 't';
	@INC = '../lib';
	require Config; import Config;
	require './test.pl';
	require './charset_tools.pl';
}

plan 11;

# open::import expects 'open' as its first argument, but it clashes with open()
sub import {
	open::import( 'open', @_ );
}

# can't use require_ok() here, with a name like 'open'
ok( require 'open.pm', 'requiring open' );

# this should fail
eval { import() };
like( $@, qr/needs explicit list of PerlIO layers/,
	'import should fail without args' );

# prevent it from loading I18N::Langinfo, so we can test encoding failures
my $warn;
local $SIG{__WARN__} = sub {
	$warn .= shift;
};

# and it shouldn't be able to find this layer
$warn = '';
eval q{ no warnings 'layer'; use open IN => ':macguffin' ; };
is( $warn, '',
	'should not warn about unknown layer with bad layer provided' );

$warn = '';
eval q{ use warnings 'layer'; use open IN => ':macguffin' ; };
like( $warn, qr/Unknown PerlIO layer/,
	'should warn about unknown layer with bad layer provided' );

# open :locale logic changed since open 1.04, new logic
# difficult to test portably.

# see if it sets the magic variables appropriately
import( 'IN', ':crlf' );
is( $^H{'open_IN'}, 'crlf', 'should have set crlf layer' );

# it should reset them appropriately, too
import( 'IN', ':raw' );
is( $^H{'open_IN'}, 'raw', 'should have reset to raw layer' );

# it dies if you don't set IN, OUT, or IO
eval { import( 'sideways', ':raw' ) };
like( $@, qr/Unknown PerlIO layer class/, 'should croak with unknown class' );

# but it handles them all so well together
import( 'IO', ':raw :crlf' );
is( ${^OPEN}, ":raw :crlf\0:raw :crlf",
	'should set multi types, multi layer' );
is( $^H{'open_IO'}, 'crlf', 'should record last layer set in %^H' );

SKIP: {
    skip("no perlio", 1) unless (find PerlIO::Layer 'perlio');
    skip("no Encode", 1) unless $Config{extensions} =~ m{\bEncode\b};
    skip("EBCDIC platform doesnt have 'use encoding' used by open ':locale'", 1)
                                                                if $::IS_EBCDIC;

    eval q[use Encode::Alias;use open ":std", ":locale"];
    is($@, '', 'can use :std and :locale');
}

{
    local $ENV{PERL_UNICODE};
    delete $ENV{PERL_UNICODE};
    local $TODO;
    $TODO = "Encode not working on EBCDIC" if $::IS_EBCDIC;
    is runperl(
         progs => [
            'use open q\:encoding(UTF-8)\, q-:std-;',
            'use open q\:encoding(UTF-8)\;',
            'if(($_ = <STDIN>) eq qq-\x{100}\n-) { print qq-stdin ok\n- }',
            'else { print qq-got -, join(q q q, map ord, split//), "\n" }',
            'print STDOUT qq-\x{fe}\n-;',
            'print STDERR qq-\x{fe}\n-;',
         ],
         stdin => byte_utf8a_to_utf8n("\xc4\x80") . "\n",
         stderr => 1,
       ),
       "stdin ok\n"
        . byte_utf8a_to_utf8n("\xc3\xbe")
        . "\n"
        . byte_utf8a_to_utf8n("\xc3\xbe")
        . "\n",
       "use open without :std does not affect standard handles",
    ;
}

END {
    1 while unlink "utf8";
    1 while unlink "a";
    1 while unlink "b";
}

# the test cases beyond __DATA__ need to be executed separately

__DATA__
$ENV{LC_ALL} = 'nonexistent.euc';
eval { open::_get_locale_encoding() };
like( $@, qr/too ambiguous/, 'should die with ambiguous locale encoding' );
%%%
# the special :locale layer
$ENV{LC_ALL} = $ENV{LANG} = 'ru_RU.KOI8-R';
# the :locale will probe the locale environment variables like LANG
use open OUT => ':locale';
open(O, ">koi8");
print O chr(0x430); # Unicode CYRILLIC SMALL LETTER A = KOI8-R 0xc1
close O;
open(I, "<koi8");
printf "%#x\n", ord(<I>), "\n"; # this should print 0xc1
close I;
%%%
