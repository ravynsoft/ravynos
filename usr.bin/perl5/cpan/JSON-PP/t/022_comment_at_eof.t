# copied over from JSON::XS and modified to use JSON::PP

# the original test case was provided by IKEGAMI@cpan.org

use strict;
use warnings;

use Test::More tests => 13;

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

use Data::Dumper qw( Dumper );

sub decoder {
   my ($str) = @_;

   my $json = JSON::PP->new->relaxed;

   $json->incr_parse($_[0]);

   my $rv;
   if (!eval { $rv = $json->incr_parse(); 1 }) {
       $rv = "died with $@";
   }

   local $Data::Dumper::Useqq = 1;
   local $Data::Dumper::Terse = 1;
   local $Data::Dumper::Indent = 0;

   return Dumper($rv);
}

is( decoder( "[]"        ), '[]', 'array baseline' );
is( decoder( " []"       ), '[]', 'space ignored before array' );
is( decoder( "\n[]"      ), '[]', 'newline ignored before array' );
is( decoder( "# foo\n[]" ), '[]', 'comment ignored before array' );
is( decoder( "# fo[o\n[]"), '[]', 'comment ignored before array' );
is( decoder( "# fo]o\n[]"), '[]', 'comment ignored before array' );
is( decoder( "[# fo]o\n]"), '[]', 'comment ignored inside array' );

is( decoder( ""        ), 'undef', 'eof baseline' );
is( decoder( " "       ), 'undef', 'space ignored before eof' );
is( decoder( "\n"      ), 'undef', 'newline ignored before eof' );
is( decoder( "#,foo\n" ), 'undef', 'comment ignored before eof' );
is( decoder( "# []o\n" ), 'undef', 'comment ignored before eof' );

is( decoder(qq/#\n[#foo\n"#\\n"#\n]/), '["#\n"]', 'array and string in multiple lines' );

