package Hints_provider_easy_does_it;
use strict;
use warnings;
use Exporter 5.57 'import';

our @EXPORT_OK = qw(always_fail always_pass no_hints);

our %DOES = ( 'autodie::hints::provider' => 1 );

my $package = __PACKAGE__;

sub AUTODIE_HINTS {
    return {
        always_fail => { list => sub { 1 }, scalar => sub { 1 } },
        always_pass => { list => sub { 0 }, scalar => sub { 0 } },
    };
}

sub always_fail { return "foo" };
sub always_pass { return "foo" };
sub no_hints    { return "foo" };

1;
