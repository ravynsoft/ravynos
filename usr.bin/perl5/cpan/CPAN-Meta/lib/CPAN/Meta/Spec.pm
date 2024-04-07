# XXX RULES FOR PATCHING THIS FILE XXX
# Patches that fix typos or formatting are acceptable.  Patches
# that change semantics are not acceptable without prior approval
# by David Golden or Ricardo Signes.

use 5.006;
use strict;
use warnings;
package CPAN::Meta::Spec;

our $VERSION = '2.150010';

1;

# ABSTRACT: specification for CPAN distribution metadata


# vi:tw=72

__END__

=pod

=encoding UTF-8

=head1 NAME

CPAN::Meta::Spec - specification for CPAN distribution metadata

=head1 VERSION

version 2.150010

=head1 SYNOPSIS

  my $distmeta = {
    name => 'Module-Build',
    abstract => 'Build and install Perl modules',
    description =>  "Module::Build is a system for "
      . "building, testing, and installing Perl modules. "
      . "It is meant to ... blah blah blah ...",
    version  => '0.36',
    release_status => 'stable',
    author   => [
      'Ken Williams <kwilliams@cpan.org>',
      'Module-Build List <module-build@perl.org>', # additional contact
    ],
    license  => [ 'perl_5' ],
    prereqs => {
      runtime => {
        requires => {
          'perl'   => '5.006',
          'ExtUtils::Install' => '0',
          'File::Basename' => '0',
          'File::Compare'  => '0',
          'IO::File'   => '0',
        },
        recommends => {
          'Archive::Tar' => '1.00',
          'ExtUtils::Install' => '0.3',
          'ExtUtils::ParseXS' => '2.02',
        },
      },
      build => {
        requires => {
          'Test::More' => '0',
        },
      }
    },
    resources => {
      license => ['http://dev.perl.org/licenses/'],
    },
    optional_features => {
      domination => {
        description => 'Take over the world',
        prereqs     => {
          develop => { requires => { 'Genius::Evil'     => '1.234' } },
          runtime => { requires => { 'Machine::Weather' => '2.0'   } },
        },
      },
    },
    dynamic_config => 1,
    keywords => [ qw/ toolchain cpan dual-life / ],
    'meta-spec' => {
      version => '2',
      url     => 'https://metacpan.org/pod/CPAN::Meta::Spec',
    },
    generated_by => 'Module::Build version 0.36',
  };

=head1 DESCRIPTION

This document describes version 2 of the CPAN distribution metadata
specification, also known as the "CPAN Meta Spec".

Revisions of this specification for typo corrections and prose
clarifications may be issued as CPAN::Meta::Spec 2.I<x>.  These
revisions will never change semantics or add or remove specified
behavior.

Distribution metadata describe important properties of Perl
distributions. Distribution building tools like Module::Build,
Module::Install, ExtUtils::MakeMaker or Dist::Zilla should create a
metadata file in accordance with this specification and include it with
the distribution for use by automated tools that index, examine, package
or install Perl distributions.

=head1 TERMINOLOGY

=over 4

=item distribution

This is the primary object described by the metadata. In the context of
this document it usually refers to a collection of modules, scripts,
and/or documents that are distributed together for other developers to
use.  Examples of distributions are C<Class-Container>, C<libwww-perl>,
or C<DBI>.

=item module

This refers to a reusable library of code contained in a single file.
Modules usually contain one or more packages and are often referred
to by the name of a primary package that can be mapped to the file
name. For example, one might refer to C<File::Spec> instead of
F<File/Spec.pm>

=item package

This refers to a namespace declared with the Perl C<package> statement.
In Perl, packages often have a version number property given by the
C<$VERSION> variable in the namespace.

=item consumer

This refers to code that reads a metadata file, deserializes it into a
data structure in memory, or interprets a data structure of metadata
elements.

=item producer

This refers to code that constructs a metadata data structure,
serializes into a bytestream and/or writes it to disk.

=item must, should, may, etc.

These terms are interpreted as described in IETF RFC 2119.

=back

=head1 DATA TYPES

Fields in the L</STRUCTURE> section describe data elements, each of
which has an associated data type as described herein.  There are four
primitive types: Boolean, String, List and Map.  Other types are
subtypes of primitives and define compound data structures or define
constraints on the values of a data element.

=head2 Boolean

A I<Boolean> is used to provide a true or false value.  It B<must> be
represented as a defined value that is either "1" or "0" or stringifies
to those values.

=head2 String

A I<String> is data element containing a non-zero length sequence of
Unicode characters, such as an ordinary Perl scalar that is not a
reference.

=head2 List

A I<List> is an ordered collection of zero or more data elements.
Elements of a List may be of mixed types.

Producers B<must> represent List elements using a data structure which
unambiguously indicates that multiple values are possible, such as a
reference to a Perl array (an "arrayref").

Consumers expecting a List B<must> consider a String as equivalent to a
List of length 1.

=head2 Map

A I<Map> is an unordered collection of zero or more data elements
("values"), indexed by associated String elements ("keys").  The Map's
value elements may be of mixed types.

=head2 License String

A I<License String> is a subtype of String with a restricted set of
values.  Valid values are described in detail in the description of
the L</license> field.

=head2 URL

I<URL> is a subtype of String containing a Uniform Resource Locator or
Identifier.  [ This type is called URL and not URI for historical reasons. ]

=head2 Version

A I<Version> is a subtype of String containing a value that describes
the version number of packages or distributions.  Restrictions on format
are described in detail in the L</Version Formats> section.

=head2 Version Range

The I<Version Range> type is a subtype of String.  It describes a range
of Versions that may be present or installed to fulfill prerequisites.
It is specified in detail in the L</Version Ranges> section.

=head1 STRUCTURE

The metadata structure is a data element of type Map.  This section
describes valid keys within the Map.

Any keys not described in this specification document (whether top-level
or within compound data structures described herein) are considered
I<custom keys> and B<must> begin with an "x" or "X" and be followed by an
underscore; i.e. they must match the pattern: C<< qr{\Ax_}i >>.  If a
custom key refers to a compound data structure, subkeys within it do not
need an "x_" or "X_" prefix.

Consumers of metadata may ignore any or all custom keys.  All other keys
not described herein are invalid and should be ignored by consumers.
Producers must not generate or output invalid keys.

For each key, an example is provided followed by a description.  The
description begins with the version of spec in which the key was added
or in which the definition was modified, whether the key is I<required>
or I<optional> and the data type of the corresponding data element.
These items are in parentheses, brackets and braces, respectively.

If a data type is a Map or Map subtype, valid subkeys will be described
as well.

Some fields are marked I<Deprecated>.  These are shown for historical
context and must not be produced in or consumed from any metadata structure
of version 2 or higher.

=head2 REQUIRED FIELDS

=head3 abstract

Example:

  abstract => 'Build and install Perl modules'

(Spec 1.2) [required] {String}

This is a short description of the purpose of the distribution.

=head3 author

Example:

  author => [ 'Ken Williams <kwilliams@cpan.org>' ]

(Spec 1.2) [required] {List of one or more Strings}

This List indicates the person(s) to contact concerning the
distribution. The preferred form of the contact string is:

  contact-name <email-address>

This field provides a general contact list independent of other
structured fields provided within the L</resources> field, such as
C<bugtracker>.  The addressee(s) can be contacted for any purpose
including but not limited to (security) problems with the distribution,
questions about the distribution or bugs in the distribution.

A distribution's original author is usually the contact listed within
this field.  Co-maintainers, successor maintainers or mailing lists
devoted to the distribution may also be listed in addition to or instead
of the original author.

=head3 dynamic_config

Example:

  dynamic_config => 1

(Spec 2) [required] {Boolean}

A boolean flag indicating whether a F<Build.PL> or F<Makefile.PL> (or
similar) must be executed to determine prerequisites.

This field should be set to a true value if the distribution performs
some dynamic configuration (asking questions, sensing the environment,
etc.) as part of its configuration.  This field should be set to a false
value to indicate that prerequisites included in metadata may be
considered final and valid for static analysis.

Note: when this field is true, post-configuration prerequisites are not
guaranteed to bear any relation whatsoever to those stated in the metadata,
and relying on them doing so is an error. See also
L</Prerequisites for dynamically configured distributions> in the implementors'
notes.

This field explicitly B<does not> indicate whether installation may be
safely performed without using a Makefile or Build file, as there may be
special files to install or custom installation targets (e.g. for
dual-life modules that exist on CPAN as well as in the Perl core).  This
field only defines whether or not prerequisites are exactly as given in the
metadata.

=head3 generated_by

Example:

  generated_by => 'Module::Build version 0.36'

(Spec 1.0) [required] {String}

This field indicates the tool that was used to create this metadata.
There are no defined semantics for this field, but it is traditional to
use a string in the form "Generating::Package version 1.23" or the
author's name, if the file was generated by hand.

=head3 license

Example:

  license => [ 'perl_5' ]

  license => [ 'apache_2_0', 'mozilla_1_0' ]

(Spec 2) [required] {List of one or more License Strings}

One or more licenses that apply to some or all of the files in the
distribution.  If multiple licenses are listed, the distribution
documentation should be consulted to clarify the interpretation of
multiple licenses.

The following list of license strings are valid:

 string          description
 -------------   -----------------------------------------------
 agpl_3          GNU Affero General Public License, Version 3
 apache_1_1      Apache Software License, Version 1.1
 apache_2_0      Apache License, Version 2.0
 artistic_1      Artistic License, (Version 1)
 artistic_2      Artistic License, Version 2.0
 bsd             BSD License (three-clause)
 freebsd         FreeBSD License (two-clause)
 gfdl_1_2        GNU Free Documentation License, Version 1.2
 gfdl_1_3        GNU Free Documentation License, Version 1.3
 gpl_1           GNU General Public License, Version 1
 gpl_2           GNU General Public License, Version 2
 gpl_3           GNU General Public License, Version 3
 lgpl_2_1        GNU Lesser General Public License, Version 2.1
 lgpl_3_0        GNU Lesser General Public License, Version 3.0
 mit             MIT (aka X11) License
 mozilla_1_0     Mozilla Public License, Version 1.0
 mozilla_1_1     Mozilla Public License, Version 1.1
 openssl         OpenSSL License
 perl_5          The Perl 5 License (Artistic 1 & GPL 1 or later)
 qpl_1_0         Q Public License, Version 1.0
 ssleay          Original SSLeay License
 sun             Sun Internet Standards Source License (SISSL)
 zlib            zlib License

The following license strings are also valid and indicate other
licensing not described above:

 string          description
 -------------   -----------------------------------------------
 open_source     Other Open Source Initiative (OSI) approved license
 restricted      Requires special permission from copyright holder
 unrestricted    Not an OSI approved license, but not restricted
 unknown         License not provided in metadata

All other strings are invalid in the license field.

=head3 meta-spec

Example:

  'meta-spec' => {
    version => '2',
    url     => 'http://search.cpan.org/perldoc?CPAN::Meta::Spec',
  }

(Spec 1.2) [required] {Map}

This field indicates the version of the CPAN Meta Spec that should be
used to interpret the metadata.  Consumers must check this key as soon
as possible and abort further metadata processing if the meta-spec
version is not supported by the consumer.

The following keys are valid, but only C<version> is required.

=over

=item version

This subkey gives the integer I<Version> of the CPAN Meta Spec against
which the document was generated.

=item url

This is a I<URL> of the metadata specification document corresponding to
the given version.  This is strictly for human-consumption and should
not impact the interpretation of the document.

For the version 2 spec, either of these are recommended:

=over 4

=item *

C<https://metacpan.org/pod/CPAN::Meta::Spec>

=item *

C<http://search.cpan.org/perldoc?CPAN::Meta::Spec>

=back

=back

=head3 name

Example:

  name => 'Module-Build'

(Spec 1.0) [required] {String}

This field is the name of the distribution.  This is often created by
taking the "main package" in the distribution and changing C<::> to
C<->, but the name may be completely unrelated to the packages within
the distribution.  For example, L<LWP::UserAgent> is distributed as part
of the distribution name "libwww-perl".

=head3 release_status

Example:

  release_status => 'stable'

(Spec 2) [required] {String}

This field provides the  release status of this distribution.  If the
C<version> field contains an underscore character, then
C<release_status> B<must not> be "stable."

The C<release_status> field B<must> have one of the following values:

=over

=item stable

This indicates an ordinary, "final" release that should be indexed by PAUSE
or other indexers.

=item testing

This indicates a "beta" release that is substantially complete, but has an
elevated risk of bugs and requires additional testing.  The distribution
should not be installed over a stable release without an explicit request
or other confirmation from a user.  This release status may also be used
for "release candidate" versions of a distribution.

=item unstable

This indicates an "alpha" release that is under active development, but has
been released for early feedback or testing and may be missing features or
may have serious bugs.  The distribution should not be installed over a
stable release without an explicit request or other confirmation from a
user.

=back

Consumers B<may> use this field to determine how to index the
distribution for CPAN or other repositories in addition to or in
replacement of heuristics based on version number or file name.

=head3 version

Example:

  version => '0.36'

(Spec 1.0) [required] {Version}

This field gives the version of the distribution to which the metadata
structure refers.

=head2 OPTIONAL FIELDS

=head3 description

Example:

    description =>  "Module::Build is a system for "
      . "building, testing, and installing Perl modules. "
      . "It is meant to ... blah blah blah ...",

(Spec 2) [optional] {String}

A longer, more complete description of the purpose or intended use of
the distribution than the one provided by the C<abstract> key.

=head3 keywords

Example:

  keywords => [ qw/ toolchain cpan dual-life / ]

(Spec 1.1) [optional] {List of zero or more Strings}

A List of keywords that describe this distribution.  Keywords
B<must not> include whitespace.

=head3 no_index

Example:

  no_index => {
    file      => [ 'My/Module.pm' ],
    directory => [ 'My/Private' ],
    package   => [ 'My::Module::Secret' ],
    namespace => [ 'My::Module::Sample' ],
  }

(Spec 1.2) [optional] {Map}

This Map describes any files, directories, packages, and namespaces that
are private to the packaging or implementation of the distribution and
should be ignored by indexing or search tools. Note that this is a list of
exclusions, and the spec does not define what to I<include> - see
L</Indexing distributions a la PAUSE> in the implementors notes for more
information.

Valid subkeys are as follows:

=over

=item file

A I<List> of relative paths to files.  Paths B<must be> specified with
unix conventions.

=item directory

A I<List> of relative paths to directories.  Paths B<must be> specified
with unix conventions.

[ Note: previous editions of the spec had C<dir> instead of C<directory> ]

=item package

A I<List> of package names.

=item namespace

A I<List> of package namespaces, where anything below the namespace
must be ignored, but I<not> the namespace itself.

In the example above for C<no_index>, C<My::Module::Sample::Foo> would
be ignored, but C<My::Module::Sample> would not.

=back

=head3 optional_features

Example:

  optional_features => {
    sqlite => {
      description => 'Provides SQLite support',
      prereqs => {
        runtime => {
          requires => {
            'DBD::SQLite' => '1.25'
          }
        }
      }
    }
  }

(Spec 2) [optional] {Map}

This Map describes optional features with incremental prerequisites.
Each key of the C<optional_features> Map is a String used to identify
the feature and each value is a Map with additional information about
the feature.  Valid subkeys include:

=over

=item description

This is a String describing the feature.  Every optional feature
should provide a description

=item prereqs

This entry is required and has the same structure as that of the
C<L</prereqs>> key.  It provides a list of package requirements
that must be satisfied for the feature to be supported or enabled.

There is one crucial restriction:  the prereqs of an optional feature
B<must not> include C<configure> phase prereqs.

=back

Consumers B<must not> include optional features as prerequisites without
explicit instruction from users (whether via interactive prompting,
a function parameter or a configuration value, etc. ).

If an optional feature is used by a consumer to add additional
prerequisites, the consumer should merge the optional feature
prerequisites into those given by the C<prereqs> key using the same
semantics.  See L</Merging and Resolving Prerequisites> for details on
merging prerequisites.

I<Suggestion for disuse:> Because there is currently no way for a
distribution to specify a dependency on an optional feature of another
dependency, the use of C<optional_feature> is discouraged.  Instead,
create a separate, installable distribution that ensures the desired
feature is available.  For example, if C<Foo::Bar> has a C<Baz> feature,
release a separate C<Foo-Bar-Baz> distribution that satisfies
requirements for the feature.

=head3 prereqs

Example:

  prereqs => {
    runtime => {
      requires => {
        'perl'          => '5.006',
        'File::Spec'    => '0.86',
        'JSON'          => '2.16',
      },
      recommends => {
        'JSON::XS'      => '2.26',
      },
      suggests => {
        'Archive::Tar'  => '0',
      },
    },
    build => {
      requires => {
        'Alien::SDL'    => '1.00',
      },
    },
    test => {
      recommends => {
        'Test::Deep'    => '0.10',
      },
    }
  }

(Spec 2) [optional] {Map}

This is a Map that describes all the prerequisites of the distribution.
The keys are phases of activity, such as C<configure>, C<build>, C<test>
or C<runtime>.  Values are Maps in which the keys name the type of
prerequisite relationship such as C<requires>, C<recommends>, or
C<suggests> and the value provides a set of prerequisite relations.  The
set of relations B<must> be specified as a Map of package names to
version ranges.

The full definition for this field is given in the L</Prereq Spec>
section.

=head3 provides

Example:

  provides => {
    'Foo::Bar' => {
      file    => 'lib/Foo/Bar.pm',
      version => '0.27_02',
    },
    'Foo::Bar::Blah' => {
      file    => 'lib/Foo/Bar/Blah.pm',
    },
    'Foo::Bar::Baz' => {
      file    => 'lib/Foo/Bar/Baz.pm',
      version => '0.3',
    },
  }

(Spec 1.2) [optional] {Map}

This describes all packages provided by this distribution.  This
information is used by distribution and automation mechanisms like
PAUSE, CPAN, metacpan.org and search.cpan.org to build indexes saying in
which distribution various packages can be found.

The keys of C<provides> are package names that can be found within
the distribution.  If a package name key is provided, it must
have a Map with the following valid subkeys:

=over

=item file

This field is required.  It must contain a Unix-style relative file path
from the root of the distribution directory to a file that contains or
generates the package.  It may be given as C<META.yml> or C<META.json>
to claim a package for indexing without needing a C<*.pm>.

=item version

If it exists, this field must contains a I<Version> String for the
package.  If the package does not have a C<$VERSION>, this field must
be omitted.

=back

=head3 resources

Example:

  resources => {
    license     => [ 'http://dev.perl.org/licenses/' ],
    homepage    => 'http://sourceforge.net/projects/module-build',
    bugtracker  => {
      web    => 'http://rt.cpan.org/Public/Dist/Display.html?Name=CPAN-Meta',
      mailto => 'meta-bugs@example.com',
    },
    repository  => {
      url  => 'git://github.com/dagolden/cpan-meta.git',
      web  => 'http://github.com/dagolden/cpan-meta',
      type => 'git',
    },
    x_twitter   => 'http://twitter.com/cpan_linked/',
  }

(Spec 2) [optional] {Map}

This field describes resources related to this distribution.

Valid subkeys include:

=over

=item homepage

The official home of this project on the web.

=item license

A List of I<URL>'s that relate to this distribution's license.  As with the
top-level C<license> field, distribution documentation should be consulted
to clarify the interpretation of multiple licenses provided here.

=item bugtracker

This entry describes the bug tracking system for this distribution.  It
is a Map with the following valid keys:

  web    - a URL pointing to a web front-end for the bug tracker
  mailto - an email address to which bugs can be sent

=item repository

This entry describes the source control repository for this distribution.  It
is a Map with the following valid keys:

  url  - a URL pointing to the repository itself
  web  - a URL pointing to a web front-end for the repository
  type - a lowercase string indicating the VCS used

Because a url like C<http://myrepo.example.com/> is ambiguous as to
type, producers should provide a C<type> whenever a C<url> key is given.
The C<type> field should be the name of the most common program used
to work with the repository, e.g. C<git>, C<svn>, C<cvs>, C<darcs>,
C<bzr> or C<hg>.

=back

=head2 DEPRECATED FIELDS

=head3 build_requires

I<(Deprecated in Spec 2)> [optional] {String}

Replaced by C<prereqs>

=head3 configure_requires

I<(Deprecated in Spec 2)> [optional] {String}

Replaced by C<prereqs>

=head3 conflicts

I<(Deprecated in Spec 2)> [optional] {String}

Replaced by C<prereqs>

=head3 distribution_type

I<(Deprecated in Spec 2)> [optional] {String}

This field indicated 'module' or 'script' but was considered
meaningless, since many distributions are hybrids of several kinds of
things.

=head3 license_uri

I<(Deprecated in Spec 1.2)> [optional] {URL}

Replaced by C<license> in C<resources>

=head3 private

I<(Deprecated in Spec 1.2)> [optional] {Map}

This field has been renamed to L</"no_index">.

=head3 recommends

I<(Deprecated in Spec 2)> [optional] {String}

Replaced by C<prereqs>

=head3 requires

I<(Deprecated in Spec 2)> [optional] {String}

Replaced by C<prereqs>

=head1 VERSION NUMBERS

=head2 Version Formats

This section defines the Version type, used by several fields in the
CPAN Meta Spec.

Version numbers must be treated as strings, not numbers.  For
example, C<1.200> B<must not> be serialized as C<1.2>.  Version
comparison should be delegated to the Perl L<version> module, version
0.80 or newer.

Unless otherwise specified, version numbers B<must> appear in one of two
formats:

=over

=item Decimal versions

Decimal versions are regular "decimal numbers", with some limitations.
They B<must> be non-negative and B<must> begin and end with a digit.  A
single underscore B<may> be included, but B<must> be between two digits.
They B<must not> use exponential notation ("1.23e-2").

   version => '1.234'       # OK
   version => '1.23_04'     # OK

   version => '1.23_04_05'  # Illegal
   version => '1.'          # Illegal
   version => '.1'          # Illegal

=item Dotted-integer versions

Dotted-integer (also known as dotted-decimal) versions consist of
positive integers separated by full stop characters (i.e. "dots",
"periods" or "decimal points").  This are equivalent in format to Perl
"v-strings", with some additional restrictions on form.  They must be
given in "normal" form, which has a leading "v" character and at least
three integer components.  To retain a one-to-one mapping with decimal
versions, all components after the first B<should> be restricted to the
range 0 to 999.  The final component B<may> be separated by an
underscore character instead of a period.

   version => 'v1.2.3'      # OK
   version => 'v1.2_3'      # OK
   version => 'v1.2.3.4'    # OK
   version => 'v1.2.3_4'    # OK
   version => 'v2009.10.31' # OK

   version => 'v1.2'          # Illegal
   version => '1.2.3'         # Illegal
   version => 'v1.2_3_4'      # Illegal
   version => 'v1.2009.10.31' # Not recommended

=back

=head2 Version Ranges

Some fields (prereq, optional_features) indicate the particular
version(s) of some other module that may be required as a prerequisite.
This section details the Version Range type used to provide this
information.

The simplest format for a Version Range is just the version
number itself, e.g. C<2.4>.  This means that B<at least> version 2.4
must be present.  To indicate that B<any> version of a prerequisite is
okay, even if the prerequisite doesn't define a version at all, use
the version C<0>.

Alternatively, a version range B<may> use the operators E<lt> (less than),
E<lt>= (less than or equal), E<gt> (greater than), E<gt>= (greater than
or equal), == (equal), and != (not equal).  For example, the
specification C<E<lt> 2.0> means that any version of the prerequisite
less than 2.0 is suitable.

For more complicated situations, version specifications B<may> be AND-ed
together using commas.  The specification C<E<gt>= 1.2, != 1.5, E<lt>
2.0> indicates a version that must be B<at least> 1.2, B<less than> 2.0,
and B<not equal to> 1.5.

=head1 PREREQUISITES

=head2 Prereq Spec

The C<prereqs> key in the top-level metadata and within
C<optional_features> define the relationship between a distribution and
other packages.  The prereq spec structure is a hierarchical data
structure which divides prerequisites into I<Phases> of activity in the
installation process and I<Relationships> that indicate how
prerequisites should be resolved.

For example, to specify that C<Data::Dumper> is C<required> during the
C<test> phase, this entry would appear in the distribution metadata:

  prereqs => {
    test => {
      requires => {
        'Data::Dumper' => '2.00'
      }
    }
  }

=head3 Phases

Requirements for regular use must be listed in the C<runtime> phase.
Other requirements should be listed in the earliest stage in which they
are required and consumers must accumulate and satisfy requirements
across phases before executing the activity. For example, C<build>
requirements must also be available during the C<test> phase.

  before action       requirements that must be met
  ----------------    --------------------------------
  perl Build.PL       configure
  perl Makefile.PL

  make                configure, runtime, build
  Build

  make test           configure, runtime, build, test
  Build test

Consumers that install the distribution must ensure that
I<runtime> requirements are also installed and may install
dependencies from other phases.

  after action        requirements that must be met
  ----------------    --------------------------------
  make install        runtime
  Build install

=over

=item configure

The configure phase occurs before any dynamic configuration has been
attempted.  Libraries required by the configure phase B<must> be
available for use before the distribution building tool has been
executed.

=item build

The build phase is when the distribution's source code is compiled (if
necessary) and otherwise made ready for installation.

=item test

The test phase is when the distribution's automated test suite is run.
Any library that is needed only for testing and not for subsequent use
should be listed here.

=item runtime

The runtime phase refers not only to when the distribution's contents
are installed, but also to its continued use.  Any library that is a
prerequisite for regular use of this distribution should be indicated
here.

=item develop

The develop phase's prereqs are libraries needed to work on the
distribution's source code as its author does.  These tools might be
needed to build a release tarball, to run author-only tests, or to
perform other tasks related to developing new versions of the
distribution.

=back

=head3 Relationships

=over

=item requires

These dependencies B<must> be installed for proper completion of the
phase.

=item recommends

Recommended dependencies are I<strongly> encouraged and should be
satisfied except in resource constrained environments.

=item suggests

These dependencies are optional, but are suggested for enhanced operation
of the described distribution.

=item conflicts

These libraries cannot be installed when the phase is in operation.
This is a very rare situation, and the C<conflicts> relationship should
be used with great caution, or not at all.

=back

=head2 Merging and Resolving Prerequisites

Whenever metadata consumers merge prerequisites, either from different
phases or from C<optional_features>, they should merged in a way which
preserves the intended semantics of the prerequisite structure.  Generally,
this means concatenating the version specifications using commas, as
described in the L<Version Ranges> section.

Another subtle error that can occur in resolving prerequisites comes from
the way that modules in prerequisites are indexed to distribution files on
CPAN.  When a module is deleted from a distribution, prerequisites calling
for that module could indicate an older distribution should be installed,
potentially overwriting files from a newer distribution.

For example, as of Oct 31, 2009, the CPAN index file contained these
module-distribution mappings:

  Class::MOP                   0.94  D/DR/DROLSKY/Class-MOP-0.94.tar.gz
  Class::MOP::Class            0.94  D/DR/DROLSKY/Class-MOP-0.94.tar.gz
  Class::MOP::Class::Immutable 0.04  S/ST/STEVAN/Class-MOP-0.36.tar.gz

Consider the case where "Class::MOP" 0.94 is installed.  If a
distribution specified "Class::MOP::Class::Immutable" as a prerequisite,
it could result in Class-MOP-0.36.tar.gz being installed, overwriting
any files from Class-MOP-0.94.tar.gz.

Consumers of metadata B<should> test whether prerequisites would result
in installed module files being "downgraded" to an older version and
B<may> warn users or ignore the prerequisite that would cause such a
result.

=head1 SERIALIZATION

Distribution metadata should be serialized (as a hashref) as
JSON-encoded data and packaged with distributions as the file
F<META.json>.

In the past, the distribution metadata structure had been packed with
distributions as F<META.yml>, a file in the YAML Tiny format (for which,
see L<YAML::Tiny>).  Tools that consume distribution metadata from disk
should be capable of loading F<META.yml>, but should prefer F<META.json>
if both are found.

=head1 NOTES FOR IMPLEMENTORS

=head2 Extracting Version Numbers from Perl Modules

To get the version number from a Perl module, consumers should use the
C<< MM->parse_version($file) >> method provided by
L<ExtUtils::MakeMaker> or L<Module::Metadata>.  For example, for the
module given by C<$mod>, the version may be retrieved in one of the
following ways:

  # via ExtUtils::MakeMaker
  my $file = MM->_installed_file_for_module($mod);
  my $version = MM->parse_version($file)

The private C<_installed_file_for_module> method may be replaced with
other methods for locating a module in C<@INC>.

  # via Module::Metadata
  my $info = Module::Metadata->new_from_module($mod);
  my $version = $info->version;

If only a filename is available, the following approach may be used:

  # via Module::Build
  my $info = Module::Metadata->new_from_file($file);
  my $version = $info->version;

=head2 Comparing Version Numbers

The L<version> module provides the most reliable way to compare version
numbers in all the various ways they might be provided or might exist
within modules.  Given two strings containing version numbers, C<$v1> and
C<$v2>, they should be converted to C<version> objects before using
ordinary comparison operators.  For example:

  use version;
  if ( version->new($v1) <=> version->new($v2) ) {
    print "Versions are not equal\n";
  }

If the only comparison needed is whether an installed module is of a
sufficiently high version, a direct test may be done using the string
form of C<eval> and the C<use> function.  For example, for module C<$mod>
and version prerequisite C<$prereq>:

  if ( eval "use $mod $prereq (); 1" ) {
    print "Module $mod version is OK.\n";
  }

If the values of C<$mod> and C<$prereq> have not been scrubbed, however,
this presents security implications.

=head2 Prerequisites for dynamically configured distributions

When C<dynamic_config> is true, it is an error to presume that the
prerequisites given in distribution metadata will have any relationship
whatsoever to the actual prerequisites of the distribution.

In practice, however, one can generally expect such prerequisites to be
one of two things:

=over 4

=item *

The minimum prerequisites for the distribution, to which dynamic configuration will only add items

=item *

Whatever the distribution configured with on the releaser's machine at release time

=back

The second case often turns out to have identical results to the first case,
albeit only by accident.

As such, consumers may use this data for informational analysis, but
presenting it to the user as canonical or relying on it as such is
invariably the height of folly.

=head2 Indexing distributions a la PAUSE

While no_index tells you what must be ignored when indexing, this spec holds
no opinion on how you should get your initial candidate list of things to
possibly index. For "normal" distributions you might consider simply indexing
the contents of lib/, but there are many fascinating oddities on CPAN and
many dists from the days when it was normal to put the main .pm file in the
root of the distribution archive - so PAUSE currently indexes all .pm and .PL
files that are not either (a) specifically excluded by no_index (b) in
C<inc>, C<xt>, or C<t> directories, or common 'mistake' directories such as
C<perl5>.

Or: If you're trying to be PAUSE-like, make sure you skip C<inc>, C<xt> and
C<t> as well as anything marked as no_index.

Also remember: If the META file contains a provides field, you shouldn't be
indexing anything in the first place - just use that.

=head1 SEE ALSO

=over 4

=item *

CPAN, L<http://www.cpan.org/>

=item *

JSON, L<http://json.org/>

=item *

YAML, L<http://www.yaml.org/>

=item *

L<CPAN>

=item *

L<CPANPLUS>

=item *

L<ExtUtils::MakeMaker>

=item *

L<Module::Build>

=item *

L<Module::Install>

=item *

L<CPAN::Meta::History::Meta_1_4>

=back

=head1 HISTORY

Ken Williams wrote the original CPAN Meta Spec (also known as the
"META.yml spec") in 2003 and maintained it through several revisions
with input from various members of the community.  In 2005, Randy
Sims redrafted it from HTML to POD for the version 1.2 release.  Ken
continued to maintain the spec through version 1.4.

In late 2009, David Golden organized the version 2 proposal review
process.  David and Ricardo Signes drafted the final version 2 spec
in April 2010 based on the version 1.4 spec and patches contributed
during the proposal process.

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
