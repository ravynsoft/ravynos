package deprecate;
use strict;
use warnings;
our $VERSION = 0.04;

# our %Config can ignore %Config::Config, e.g. for testing
our %Config;
unless (%Config) { require Config; *Config = \%Config::Config; }

# This isn't a public API. It's internal to code maintained by the perl-porters
# If you would like it to be a public API, please send a patch with
# documentation and tests. Until then, it may change without warning.
sub __loaded_from_core {
    my ($package, $file, $expect_leaf) = @_;

    foreach my $pair ([qw(sitearchexp archlibexp)],
		      [qw(sitelibexp privlibexp)]) {
	my ($site, $priv) = @Config{@$pair};
	if ($^O eq 'VMS') {
	    for my $d ($site, $priv) { $d = VMS::Filespec::unixify($d) };
	}
	# Just in case anyone managed to configure with trailing /s
	s!/*$!!g foreach $site, $priv;

	next if $site eq $priv;
	if (uc("$priv/$expect_leaf") eq uc($file)) {
	    return 1;
	}
    }
    return 0;
}

sub import {
    my ($package, $file) = caller;

    my $expect_leaf = "$package.pm";
    $expect_leaf =~ s!::!/!g;

    if (__loaded_from_core($package, $file, $expect_leaf)) {
	my $call_depth=1;
	my @caller;
	while (@caller = caller $call_depth++) {
	    last if $caller[7]			# use/require
		and $caller[6] eq $expect_leaf;	# the package file
	}
	unless (@caller) {
	    require Carp;
	    Carp::cluck(<<"EOM");
Can't find use/require $expect_leaf in caller stack
EOM
	    return;
	}

	# This is fragile, because it
	# is directly poking in the internals of warnings.pm
	my ($call_file, $call_line, $callers_bitmask) = @caller[1,2,9];

	if (defined $callers_bitmask
	    && (vec($callers_bitmask, $warnings::Offsets{deprecated}, 1)
		|| vec($callers_bitmask, $warnings::Offsets{all}, 1))) {
	    warn <<"EOM";
$package will be removed from the Perl core distribution in the next major release. Please install it from CPAN. It is being used at $call_file, line $call_line.
EOM
	}
    }
}

1;

__END__

=head1 NAME

deprecate - Perl pragma for deprecating the inclusion of a module in core

=head1 SYNOPSIS

    use deprecate;  # warn about future absence if loaded from core


=head1 DESCRIPTION

This pragma simplifies the maintenance of dual-life modules that will no longer
be included in the Perl core in a future Perl release, but are still included
currently.

The purpose of the pragma is to alert users to the status of such a module by
issuing a warning that encourages them to install the module from CPAN, so that
a future upgrade to a perl which omits the module will not break their code.

This warning will only be issued if the module was loaded from a core library
directory, which allows the C<use deprecate> line to be included in the CPAN
version of the module. Because the pragma remains silent when the module is run
from a non-core library directory, the pragma call does not need to be patched
into or out of either the core or CPAN version of the module. The exact same
code can be shipped for either purpose.

=head2 Important Caveat

Note that when a module installs from CPAN to a core library directory rather
than the site library directories, the user gains no protection from having
installed it.

At the same time, this pragma cannot detect when such a module has installed
from CPAN to the core library, and so it would endlessly and uselessly exhort
the user to upgrade.

Therefore modules that can install from CPAN to the core library must make sure
not to call this pragma when they have done so. Generally this means that the
exact logic from the installer must be mirrored inside the module. E.g.:

    # Makefile.PL
    WriteMakefile(
        # ...
        INSTALLDIRS => ( "$]" >= 5.011 ? 'site' : 'perl' ),
    );

    # lib/Foo/Bar.pm
    use if "$]" >= 5.011, 'deprecate';

(The above example shows the most important case of this: when the target is
a Perl older than 5.12 (where the core library directories take precedence over
the site library directories) and the module being installed was included in
core in that Perl version. Under those circumstances, an upgrade of the module
from CPAN is only possible by installing to the core library.)


=head1 EXPORT

None by default.  The only method is C<import>, called by C<use deprecate;>.


=head1 SEE ALSO

First example to C<use deprecate;> was L<Switch>.


=head1 AUTHOR

Original version by Nicholas Clark


=head1 COPYRIGHT AND LICENSE

Copyright (C) 2009, 2011

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.10.0 or,
at your option, any later version of Perl 5 you may have available.


=cut
