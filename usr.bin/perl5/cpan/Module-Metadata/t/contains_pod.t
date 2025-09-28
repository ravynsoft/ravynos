use strict;
use warnings;
use Test::More tests => 3;
use Module::Metadata;

BEGIN {
  *fh_from_string = "$]" < 5.008
    ? require IO::Scalar && sub ($) {
      IO::Scalar->new(\$_[0]);
    }
    # hide in an eval'd string so Perl::MinimumVersion doesn't clutch its pearls
    : eval <<'EVAL'
    sub ($) {
      open my $fh, '<', \$_[0];
      $fh
    }
EVAL
  ;
}

{
    my $src = <<'...';
package Foo;
1;
...

    my $fh = fh_from_string($src);
    my $module = Module::Metadata->new_from_handle($fh, 'Foo.pm');
    ok(!$module->contains_pod(), 'This module does not contains POD');
}

{
    my $src = <<'...';
package Foo;
1;

=head1 NAME

Foo - bar
...

    my $fh = fh_from_string($src);
    my $module = Module::Metadata->new_from_handle($fh, 'Foo.pm');
    ok($module->contains_pod(), 'This module contains POD');
}

{
    my $src = <<'...';
package Foo;
1;

=head1 NAME

Foo - bar

=head1 AUTHORS

Tokuhiro Matsuno
...

    my $fh = fh_from_string($src);
    my $module = Module::Metadata->new_from_handle($fh, 'Foo.pm');
    ok($module->contains_pod(), 'This module contains POD');
}
