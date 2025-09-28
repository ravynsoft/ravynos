use strict;
use warnings;
use Test2::Tools::Tiny;

BEGIN {
    package Foo::Bar;
    use Test2::Util::HashBase qw/foo bar baz/;
    use Carp qw/croak/;

    sub init {
        my $self = shift;
        croak "'foo' is a required attribute"
            unless $self->{+FOO};
    }
}

skip_all("known to fail on $]") if $] le "5.006002";

$@ = "";
my ($file, $line) = (__FILE__, __LINE__ + 1);
eval { my $one = Foo::Bar->new };
my $err = $@;

like(
    $err,
    qr/^'foo' is a required attribute at \Q$file\E line $line/,
    "Croak does not report to HashBase from init"
);

done_testing;
