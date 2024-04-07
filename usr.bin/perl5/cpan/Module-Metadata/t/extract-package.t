use strict;
use warnings;
# vim:ts=8:sw=2:et:sta:sts=2

use Test::More 0.88;
use Module::Metadata;

use lib 't/lib';
use GeneratePackage;

# parse package names
# format: {
#   name => test name
#   code => code snippet (string)
#   package => expected package names
# }
my @pkg_names = (
{
  name => 'package NAME',
  package => [ 'Simple' ],
  code => <<'---',
package Simple;
---
},
{
  name => 'package NAME::SUBNAME',
  package => [ 'Simple::Edward' ],
  code => <<'---',
package Simple::Edward;
---
},
{
  name => 'package NAME::SUBNAME::',
  package => [ 'Simple::Edward::' ],
  code => <<'---',
package Simple::Edward::;
---
},
{
  name => "package NAME'SUBNAME",
  package => [ "Simple'Edward" ],
  code => <<'---',
package Simple'Edward;
---
},
{
  name => "package NAME'SUBNAME::",
  package => [ "Simple'Edward::" ],
  code => <<'---',
package Simple'Edward::;
---
},
{
  name => 'package NAME::::SUBNAME',
  package => [ 'Simple::::Edward' ],
  code => <<'---',
package Simple::::Edward;
---
},
{
  name => 'package ::NAME::SUBNAME',
  package => [ '::Simple::Edward' ],
  code => <<'---',
package ::Simple::Edward;
---
},
{
  name => 'package NAME:SUBNAME (fail)',
  package => [ 'main' ],
  code => <<'---',
package Simple:Edward;
---
},
{
  name => "package NAME' (fail)",
  package => [ 'main' ],
  code => <<'---',
package Simple';
---
},
{
  name => "package NAME::SUBNAME' (fail)",
  package => [ 'main' ],
  code => <<'---',
package Simple::Edward';
---
},
{
  name => "package NAME''SUBNAME (fail)",
  package => [ 'main' ],
  code => <<'---',
package Simple''Edward;
---
},
{
  name => 'package NAME-SUBNAME (fail)',
  package => [ 'main' ],
  code => <<'---',
package Simple-Edward;
---
},
{
  name => 'no assumption of package merely if its $VERSION is referenced',
  package => [ 'Simple' ],
  code => <<'---',
package Simple;
$Foo::Bar::VERSION = '1.23';
---
},
{
  name => 'script 7 from t/metadata.t', # TODO merge these
  package => [ '_private', 'main' ],
  TODO => '$::VERSION indicates main namespace is referenced',
  code => <<'---',
package _private;
$::VERSION = 0.01;
$VERSION = '999';
---
},
{
  name => 'script 8 from t/metadata.t', # TODO merge these
  package => [ '_private', 'main' ],
  TODO => '$::VERSION indicates main namespace is referenced',
  code => <<'---',
package _private;
$VERSION = '999';
$::VERSION = 0.01;
---
},
);

my $test_num = 0;

my $tmpdir = GeneratePackage::tmpdir();

foreach my $test_case (@pkg_names) {
    note '-------';
    note $test_case->{name};
    my $code = $test_case->{code};
    my $expected_name = $test_case->{package};

    my $warnings = '';
    local $SIG{__WARN__} = sub { $warnings .= $_ for @_ };

    my $pm_info = Module::Metadata->new_from_file(generate_file(File::Spec->catfile($tmpdir, "Simple${test_num}"), 'Simple.pm', $code));

    # whenever we drop support for 5.6, we can do this:
    # open my $fh, '<', \(encode('UTF-8', $code, Encode::FB_CROAK))
    #     or die "cannot open handle to code string: $!";
    # my $pm_info = Module::Metadata->new_from_handle($fh, 'lib/Simple.pm');

    # Test::Builder will prematurely numify objects, so use this form
    my $errs;
    my @got = $pm_info->packages_inside();
  {
    local $TODO = $test_case->{TODO};
    is_deeply( \@got, $expected_name,
               "case $test_case->{name}: correct package names (expected '" . join(', ', @$expected_name) . "')" )
            or $errs++;
  }
    is( $warnings, '', "case $test_case->{name}: no warnings from parsing" ) or $errs++;
    diag "Got: '" . join(', ', @got) . "'\nModule contents:\n$code"
      if $errs and not $ENV{PERL_CORE} and ($ENV{AUTHOR_TESTING} or $ENV{AUTOMATED_TESTING});
}
continue {
  ++$test_num;
}

done_testing;
