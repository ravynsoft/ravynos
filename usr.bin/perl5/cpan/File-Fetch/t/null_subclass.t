use strict;
use warnings;

use Test::More tests => 5;

my $parent_class = 'File::Fetch';
my $child_class  = 'File::Fetch::Subclass';

use_ok( $parent_class );

my $ff_parent = $parent_class->new( uri => 'http://example.com/index.html' );
isa_ok( $ff_parent, $parent_class );

can_ok( $child_class, qw( new fetch ) );
my $ff_child = $child_class->new( uri => 'http://example.com/index.html' );
isa_ok( $ff_child, $child_class );
isa_ok( $ff_child, $parent_class );

BEGIN {
	package File::Fetch::Subclass;
	use vars qw(@ISA);
	unshift @ISA, qw(File::Fetch);
	}
