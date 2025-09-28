package Test2::API::Breakage;
use strict;
use warnings;

our $VERSION = '1.302194';


use Test2::Util qw/pkg_to_file/;

our @EXPORT_OK = qw{
    upgrade_suggested
    upgrade_required
    known_broken
};
BEGIN { require Exporter; our @ISA = qw(Exporter) }

sub upgrade_suggested {
    return (
        'Test::Exception'    => '0.42',
        'Test::FITesque'     => '0.04',
        'Test::Module::Used' => '0.2.5',
        'Test::Moose::More'  => '0.025',
    );
}

sub upgrade_required {
    return (
        'Test::Builder::Clutch'   => '0.07',
        'Test::Dist::VersionSync' => '1.1.4',
        'Test::Modern'            => '0.012',
        'Test::SharedFork'        => '0.34',
        'Test::Alien'             => '0.04',
        'Test::UseAllModules'     => '0.14',
        'Test::More::Prefix'      => '0.005',

        'Test2::Tools::EventDumper' => 0.000007,
        'Test2::Harness'            => 0.000013,

        'Test::DBIx::Class::Schema'    => '1.0.9',
        'Test::Clustericious::Cluster' => '0.30',
    );
}

sub known_broken {
    return (
        'Net::BitTorrent'       => '0.052',
        'Test::Able'            => '0.11',
        'Test::Aggregate'       => '0.373',
        'Test::Flatten'         => '0.11',
        'Test::Group'           => '0.20',
        'Test::ParallelSubtest' => '0.05',
        'Test::Pretty'          => '0.32',
        'Test::Wrapper'         => '0.3.0',

        'Log::Dispatch::Config::TestLog' => '0.02',
    );
}

# Not reportable:
# Device::Chip => 0.07   - Tests will not pass, but not broken if already installed, also no fixed version we can upgrade to.

sub report {
    my $class = shift;
    my ($require) = @_;

    my %suggest  = __PACKAGE__->upgrade_suggested();
    my %required = __PACKAGE__->upgrade_required();
    my %broken   = __PACKAGE__->known_broken();

    my @warn;
    for my $mod (keys %suggest) {
        my $file = pkg_to_file($mod);
        next unless $INC{$file} || ($require && eval { require $file; 1 });
        my $want = $suggest{$mod};
        next if eval { $mod->VERSION($want); 1 };
        my $error = $@;
        chomp $error;
        push @warn => " * Module '$mod' is outdated, we recommed updating above $want. error was: '$error'; INC is $INC{$file}";
    }

    for my $mod (keys %required) {
        my $file = pkg_to_file($mod);
        next unless $INC{$file} || ($require && eval { require $file; 1 });
        my $want = $required{$mod};
        next if eval { $mod->VERSION($want); 1 };
        push @warn => " * Module '$mod' is outdated and known to be broken, please update to $want or higher.";
    }

    for my $mod (keys %broken) {
        my $file = pkg_to_file($mod);
        next unless $INC{$file} || ($require && eval { require $file; 1 });
        my $tested = $broken{$mod};
        push @warn => " * Module '$mod' is known to be broken in version $tested and below, newer versions have not been tested. You have: " . $mod->VERSION;
    }

    return @warn;
}

1;

__END__


=pod

=encoding UTF-8

=head1 NAME

Test2::API::Breakage - What breaks at what version

=head1 DESCRIPTION

This module provides lists of modules that are broken, or have been broken in
the past, when upgrading L<Test::Builder> to use L<Test2>.

=head1 FUNCTIONS

These can be imported, or called as methods on the class.

=over 4

=item %mod_ver = upgrade_suggested()

=item %mod_ver = Test2::API::Breakage->upgrade_suggested()

This returns key/value pairs. The key is the module name, the value is the
version number. If the installed version of the module is at or below the
specified one then an upgrade would be a good idea, but not strictly necessary.

=item %mod_ver = upgrade_required()

=item %mod_ver = Test2::API::Breakage->upgrade_required()

This returns key/value pairs. The key is the module name, the value is the
version number. If the installed version of the module is at or below the
specified one then an upgrade is required for the module to work properly.

=item %mod_ver = known_broken()

=item %mod_ver = Test2::API::Breakage->known_broken()

This returns key/value pairs. The key is the module name, the value is the
version number. If the installed version of the module is at or below the
specified one then the module will not work. A newer version may work, but is
not tested or verified.

=back

=head1 SOURCE

The source code repository for Test2 can be found at
F<http://github.com/Test-More/test-more/>.

=head1 MAINTAINERS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 AUTHORS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 COPYRIGHT

Copyright 2020 Chad Granum E<lt>exodist@cpan.orgE<gt>.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See F<http://dev.perl.org/licenses/>

=cut
