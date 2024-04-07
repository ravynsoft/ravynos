=pod

This is an example of a serious design flaw in CPAN::Meta::YAML.

It is generally a bad idea to fake out a parsing process. In this case,
CPAN::Meta::YAML strips off all lines that look like comments.

It should be easy to find dozens of bugs by looking at the assumptions the code
makes and then finding a counter case like this.

That's why I'm working on Tiny::YAML and YAML::Pegex, which are driven by a
grammar.

=cut

use Test::More tests => 1;
use CPAN::Meta::YAML;

TODO: {
local $TODO = "Failing tests for things CPAN::Meta::YAML does wrong";

my $yaml = <<'...';
- |
  # Perl sub:
  sub foo {}
...
is Load($yaml)->[0],
    "# Perl sub:\nsub foo{}\n",
    "Comments in literal scalars";
}
