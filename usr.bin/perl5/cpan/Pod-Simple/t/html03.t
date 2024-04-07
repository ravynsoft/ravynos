# Testing HTML titles

BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}
 
use strict;
use warnings;
use Test;
BEGIN { plan tests => 7 };

#use Pod::Simple::Debug (10);

use Pod::Simple::HTML;

sub x { Pod::Simple::HTML->_out(
  #sub{  $_[0]->bare_output(1)  },
  "=pod\n\n$_[0]",
) }

# make sure empty file => empty output

ok 1;
ok( x(''),'', "Contentlessness" );
ok( x(qq{=pod\n\nThis is a paragraph}) =~ m{<title></title>}i );
ok( x(qq{This is a paragraph}) =~ m{<title></title>}i );
ok( x(qq{=head1 Prok\n\nThis is a paragraph}) =~ m{<title>Prok</title>}i );
ok( x(qq{=head1 NAME\n\nProk -- stuff\n\nThis}), q{/<title>Prok</title>/} );

print "# And one for the road...\n";
ok 1;

