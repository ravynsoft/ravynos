# vi:tw=72
use 5.006;
use strict;
use warnings;
package CPAN::Meta::History;

our $VERSION = '2.150010';

1;

# ABSTRACT: history of CPAN Meta Spec changes

__END__

=pod

=encoding UTF-8

=head1 NAME

CPAN::Meta::History - history of CPAN Meta Spec changes

=head1 VERSION

version 2.150010

=head1 DESCRIPTION

The CPAN Meta Spec has gone through several iterations.  It was
originally written in HTML and later revised into POD (though published
in HTML generated from the POD).  Fields were added, removed or changed,
sometimes by design and sometimes to reflect real-world usage after the
fact.

This document reconstructs the history of the CPAN Meta Spec based on
change logs, repository commit messages and the published HTML files.
In some cases, particularly prior to version 1.2, the exact version
when certain fields were introduced or changed is inconsistent between
sources.  When in doubt, the published HTML files for versions 1.0 to
1.4 as they existed when version 2 was developed are used as the
definitive source.

Starting with version 2, the specification document is part of the
CPAN-Meta distribution and will be published on CPAN as
L<CPAN::Meta::Spec>.

Going forward, specification version numbers will be integers and
decimal portions will correspond to a release date for the CPAN::Meta
library.

=head1 HISTORY

=head2 Version 2

April 2010

=over

=item *

Revised spec examples as perl data structures rather than YAML

=item *

Switched to JSON serialization from YAML

=item *

Specified allowed version number formats

=item *

Replaced 'requires', 'build_requires', 'configure_requires',
'recommends' and 'conflicts' with new 'prereqs' data structure divided
by I<phase> (configure, build, test, runtime, etc.) and I<relationship>
(requires, recommends, suggests, conflicts)

=item *

Added support for 'develop' phase for requirements for maintaining
a list of authoring tools

=item *

Changed 'license' to a list and revised the set of valid licenses

=item *

Made 'dynamic_config' mandatory to reduce confusion

=item *

Changed 'resources' subkey 'repository' to a hash that clarifies
repository type, url for browsing and url for checkout

=item *

Changed 'resources' subkey 'bugtracker' to a hash for either web
or mailto resource

=item *

Changed specification of 'optional_features':

=over

=item *

Added formal specification and usage guide instead of just example

=item *

Changed to use new prereqs data structure instead of individual keys

=back

=item *

Clarified intended use of 'author' as generalized contact list

=item *

Added 'release_status' field to indicate stable, testing or unstable
status to provide hints to indexers

=item *

Added 'description' field for a longer description of the distribution

=item *

Formalized use of "x_" or "X_" for all custom keys not listed in the
official spec

=back

=head2 Version 1.4

June 2008

=over

=item *

Noted explicit support for 'perl' in prerequisites

=item *

Added 'configure_requires' prerequisite type

=item *

Changed 'optional_features'

=over

=item *

Example corrected to show map of maps instead of list of maps
(though descriptive text said 'map' even in v1.3)

=item *

Removed 'requires_packages', 'requires_os' and 'excluded_os'
as valid subkeys

=back

=back

=head2 Version 1.3

November 2006

=over

=item *

Added 'no_index' subkey 'directory' and removed 'dir' to match actual
usage in the wild

=item *

Added a 'repository' subkey to 'resources'

=back

=head2 Version 1.2

August 2005

=over

=item *

Re-wrote and restructured spec in POD syntax

=item *

Changed 'name' to be mandatory

=item *

Changed 'generated_by' to be mandatory

=item *

Changed 'license' to be mandatory

=item *

Added version range specifications for prerequisites

=item *

Added required 'abstract' field

=item *

Added required 'author' field

=item *

Added required 'meta-spec' field to define 'version' (and 'url') of the
CPAN Meta Spec used for metadata

=item *

Added 'provides' field

=item *

Added 'no_index' field and deprecated 'private' field.  'no_index'
subkeys include 'file', 'dir', 'package' and 'namespace'

=item *

Added 'keywords' field

=item *

Added 'resources' field with subkeys 'homepage', 'license', and
'bugtracker'

=item *

Added 'optional_features' field as an alternate under 'recommends'.
Includes 'description', 'requires', 'build_requires', 'conflicts',
'requires_packages', 'requires_os' and 'excluded_os' as valid subkeys

=item *

Removed 'license_uri' field

=back

=head2 Version 1.1

May 2003

=over

=item *

Changed 'version' to be mandatory

=item *

Added 'private' field

=item *

Added 'license_uri' field

=back

=head2 Version 1.0

March 2003

=over

=item *

Original release (in HTML format only)

=item *

Included 'name', 'version', 'license', 'distribution_type', 'requires',
'recommends', 'build_requires', 'conflicts', 'dynamic_config',
'generated_by'

=back

=head1 AUTHORS

=over 4

=item *

David Golden <dagolden@cpan.org>

=item *

Ricardo Signes <rjbs@cpan.org>

=item *

Adam Kennedy <adamk@cpan.org>

=back

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2010 by David Golden, Ricardo Signes, Adam Kennedy and Contributors.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut
