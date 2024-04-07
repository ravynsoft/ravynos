package # hide from indexers
  TypemapTest::Foo;
use strict;
use warnings;
require ExtUtils::Typemaps;
our @ISA = qw(ExtUtils::Typemaps);

sub new {
  my $class = shift;
  my $obj = $class->SUPER::new(@_);
  $obj->add_typemap(ctype => 'myfoo*', xstype => 'T_PV');
  return $obj;
}

1;
