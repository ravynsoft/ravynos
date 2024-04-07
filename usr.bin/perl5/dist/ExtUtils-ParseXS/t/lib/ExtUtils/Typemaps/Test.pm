package # hide from indexers
  ExtUtils::Typemaps::Test;
use strict;
use warnings;
require ExtUtils::Typemaps;
our @ISA = qw(ExtUtils::Typemaps);

sub new {
  my $class = shift;
  my $obj = $class->SUPER::new(@_);
  $obj->add_typemap(ctype => 'mytype*', xstype => 'T_SV');
  return $obj;
}

1;
