#!../miniperl

use strict;
use warnings;

local $ENV{LC_ALL} = 'C';

my $Quiet;
@ARGV = grep { not($_ eq '-q' and $Quiet = 1) } @ARGV;

if (@ARGV) {
    my $workdir = shift;
    chdir $workdir
        or die "Couldn't chdir to '$workdir': $!";
}
require './regen/regen_lib.pl';

# MANIFEST itself is Unix style filenames, so we have to assume that Unix style
# filenames will work.

open MANIFEST, '<', 'MANIFEST'
    or die "Can't open MANIFEST: $!";
my @files =
    grep !m#/perl.*\.pod#,
    grep m#(?:\.pm|\.pod|_pm\.PL)#,
    map {s/\s.*//s; $_}
    grep { m#^(lib|ext|dist|cpan)/# && !m#/(?:t|demo|corpus)/# }
    <MANIFEST>;
close MANIFEST
    or die "$0: failed to close MANIFEST: $!";

my $out = open_new('pod/perlmodlib.pod', undef,
                   {by => "$0 extracting documentation",
                    from => 'the Perl source files'}, 1);

my %exceptions = (
    'abbrev' => 'Text::Abbrev',
    'carp' => 'Carp',
    'getopt' => 'Getopt::Std',
    'Encode::MIME::NAME' => 'Encode::MIME::Name',
    'libnetFAQ' => 'Net::libnetFAQ',
);

my (@pragma, @mod);

for my $filename (@files) {
    unless (open MOD, '<', $filename) {
        warn "Couldn't open $filename: $!";
	next;
    }

    my ($name, $thing);
    my $foundit = 0;
    {
	local $/ = "";
	while (<MOD>) {
	    next unless /^=head1 NAME/;
	    $foundit++;
	    last;
	}
    }
    unless ($foundit) {
        next if pod_for_module_has_head1_NAME($filename);
        die "p5p-controlled module $filename missing =head1 NAME\n"
            if $filename !~ m{^(dist/|cpan/)}n # under our direct control
            && $filename !~ m{/_[^/]+\z}       # not private
            && $filename ne 'lib/meta_notation.pm'      # no pod
            && $filename ne 'lib/overload/numbers.pm';  # no pod
        warn "$filename missing =head1 NAME\n" unless $Quiet;
	next;
    }
    my $title = <MOD>;
    chomp $title;
    close MOD
        or die "Error closing $filename: $!";

    ($name, $thing) = split /\s+--?\s+/, $title, 2;

    unless ($name and $thing) {
	warn "$filename missing name\n"  unless $name;
	warn "$filename missing thing\n" unless $thing or $Quiet;
	next;
    }

    $name =~ s/[^A-Za-z0-9_:\$<>].*//;
    $name = $exceptions{$name} || $name;
    $thing =~ s/^perl pragma to //i;
    $thing = ucfirst $thing;
    $title = "=item $name\n\n$thing\n\n";

    if ($name =~ /[A-Z]/) {
	push @mod, $title;
    } else {
	push @pragma, $title;
    }
}

sub pod_for_module_has_head1_NAME {
    my ($filename) = @_;
    (my $pod_file = $filename) =~ s/\.pm\z/.pod/ or return 0;
    return 0 if !-e $pod_file;
    open my $fh, '<', $pod_file
        or die "Can't open $pod_file for reading: $!\n";
    local $/ = '';
    while (my $para = <$fh>) {
        return 1 if $para =~ /\A=head1 NAME$/m;
    }
    return 0;
}

# Much easier to special case it like this than special case the depending on
# and parsing lib/Config.pod, or special case opening configpm and finding its
# =head1 (which is not found with the $/="" above)
push @mod, "=item Config\n\nAccess Perl configuration information\n\n";


# The intent of using =cut as the heredoc terminator is to make the whole file
# parse as (reasonably) sane Pod as-is to anything that attempts to
# brute-force treat it as such. The content is already useful - this just
# makes it tidier, by stopping anything doing this mistaking the rest of the
# Perl code for Pod. eg https://metacpan.org/pod/perlmodlib

print $out <<'=cut';
=head1 NAME

perlmodlib - constructing new Perl modules and finding existing ones

=head1 THE PERL MODULE LIBRARY

Many modules are included in the Perl distribution.  These are described
below, and all end in F<.pm>.  You may discover compiled library
files (usually ending in F<.so>) or small pieces of modules to be
autoloaded (ending in F<.al>); these were automatically generated
by the installation process.  You may also discover files in the
library directory that end in either F<.pl> or F<.ph>.  These are
old libraries supplied so that old programs that use them still
run.  The F<.pl> files will all eventually be converted into standard
modules, and the F<.ph> files made by B<h2ph> will probably end up
as extension modules made by B<h2xs>.  (Some F<.ph> values may
already be available through the POSIX, Errno, or Fcntl modules.)
The B<pl2pm> file in the distribution may help in your conversion,
but it's just a mechanical process and therefore far from bulletproof.

=head2 Pragmatic Modules

They work somewhat like compiler directives (pragmata) in that they
tend to affect the compilation of your program, and thus will usually
work well only when used within a C<use>, or C<no>.  Most of these
are lexically scoped, so an inner BLOCK may countermand them
by saying:

    no integer;
    no strict 'refs';
    no warnings;

which lasts until the end of that BLOCK.

Some pragmas are lexically scoped--typically those that affect the
C<$^H> hints variable.  Others affect the current package instead,
like C<use vars> and C<use subs>, which allow you to predeclare a
variables or subroutines within a particular I<file> rather than
just a block.  Such declarations are effective for the entire file
for which they were declared.  You cannot rescind them with C<no
vars> or C<no subs>.

The following pragmas are defined (and have their own documentation).

=over 12

=cut

print $out $_ for sort @pragma;

print $out <<'=cut';

=back

=head2 Standard Modules

Standard, bundled modules are all expected to behave in a well-defined
manner with respect to namespace pollution because they use the
Exporter module.  See their own documentation for details.

It's possible that not all modules listed below are installed on your
system. For example, the GDBM_File module will not be installed if you
don't have the gdbm library.

=over 12

=cut

print $out $_ for sort @mod;

print $out <<'=cut', "=cut\n";

=back

To find out I<all> modules installed on your system, including
those without documentation or outside the standard release,
just use the following command (under the default win32 shell,
double quotes should be used instead of single quotes).

    % perl -MFile::Find=find -MFile::Spec::Functions -Tlwe \
      'find { wanted => sub { print canonpath $_ if /\.pm\z/ },
      no_chdir => 1 }, @INC'

(The -T is here to prevent @INC from being populated by C<PERL5LIB>,
C<PERLLIB>, and C<PERL_USE_UNSAFE_INC>.)
They should all have their own documentation installed and accessible
via your system man(1) command.  If you do not have a B<find>
program, you can use the Perl B<find2perl> program instead, which
generates Perl code as output you can run through perl.  If you
have a B<man> program but it doesn't find your modules, you'll have
to fix your manpath.  See L<perl> for details.  If you have no
system B<man> command, you might try the B<perldoc> program.

Note also that the command C<perldoc perllocal> gives you a (possibly
incomplete) list of the modules that have been further installed on
your system. (The perllocal.pod file is updated by the standard MakeMaker
install process.)

=head2 Extension Modules

Extension modules are written in C (or a mix of Perl and C).  They
are usually dynamically loaded into Perl if and when you need them,
but may also be linked in statically.  Supported extension modules
include Socket, Fcntl, and POSIX.

Many popular C extension modules do not come bundled (at least, not
completely) due to their sizes, volatility, or simply lack of time
for adequate testing and configuration across the multitude of
platforms on which Perl was beta-tested.  You are encouraged to
look for them on CPAN (described below), or using web search engines
like Google or DuckDuckGo.

=head1 CPAN

CPAN stands for Comprehensive Perl Archive Network; it's a globally
replicated trove of Perl materials, including documentation, style
guides, tricks and traps, alternate ports to non-Unix systems and
occasional binary distributions for these.   Search engines for
CPAN can be found at https://www.cpan.org/

Most importantly, CPAN includes around a thousand unbundled modules,
some of which require a C compiler to build.  Major categories of
modules are:

=over

=item *

Language Extensions and Documentation Tools

=item *

Development Support

=item *

Operating System Interfaces

=item *

Networking, Device Control (modems) and InterProcess Communication

=item *

Data Types and Data Type Utilities

=item *

Database Interfaces

=item *

User Interfaces

=item *

Interfaces to / Emulations of Other Programming Languages

=item *

File Names, File Systems and File Locking (see also File Handles)

=item *

String Processing, Language Text Processing, Parsing, and Searching

=item *

Option, Argument, Parameter, and Configuration File Processing

=item *

Internationalization and Locale

=item *

Authentication, Security, and Encryption

=item *

World Wide Web, HTML, HTTP, CGI, MIME

=item *

Server and Daemon Utilities

=item *

Archiving and Compression

=item *

Images, Pixmap and Bitmap Manipulation, Drawing, and Graphing

=item *

Mail and Usenet News

=item *

Control Flow Utilities (callbacks and exceptions etc)

=item *

File Handle and Input/Output Stream Utilities

=item *

Miscellaneous Modules

=back

You can find the CPAN online at L<https://www.cpan.org/>

=head1 Modules: Creation, Use, and Abuse

(The following section is borrowed directly from Tim Bunce's modules
file, available at your nearest CPAN site.)

Perl implements a class using a package, but the presence of a
package doesn't imply the presence of a class.  A package is just a
namespace.  A class is a package that provides subroutines that can be
used as methods.  A method is just a subroutine that expects, as its
first argument, either the name of a package (for "static" methods),
or a reference to something (for "virtual" methods).

A module is a file that (by convention) provides a class of the same
name (sans the .pm), plus an import method in that class that can be
called to fetch exported symbols.  This module may implement some of
its methods by loading dynamic C or C++ objects, but that should be
totally transparent to the user of the module.  Likewise, the module
might set up an AUTOLOAD function to slurp in subroutine definitions on
demand, but this is also transparent.  Only the F<.pm> file is required to
exist.  See L<perlsub>, L<perlobj>, and L<AutoLoader> for details about
the AUTOLOAD mechanism.

=head2 Guidelines for Module Creation

=over 4

=item  *

Do similar modules already exist in some form?

If so, please try to reuse the existing modules either in whole or
by inheriting useful features into a new class.  If this is not
practical try to get together with the module authors to work on
extending or enhancing the functionality of the existing modules.
A perfect example is the plethora of packages in perl4 for dealing
with command line options.

If you are writing a module to expand an already existing set of
modules, please coordinate with the author of the package.  It
helps if you follow the same naming scheme and module interaction
scheme as the original author.

=item  *

Try to design the new module to be easy to extend and reuse.

Try to C<use warnings;> (or C<use warnings qw(...);>).
Remember that you can add C<no warnings qw(...);> to individual blocks
of code that need less warnings.

Use blessed references.  Use the two argument form of bless to bless
into the class name given as the first parameter of the constructor,
e.g.,:

 sub new {
     my $class = shift;
     return bless {}, $class;
 }

or even this if you'd like it to be used as either a static
or a virtual method.

 sub new {
     my $self  = shift;
     my $class = ref($self) || $self;
     return bless {}, $class;
 }

Pass arrays as references so more parameters can be added later
(it's also faster).  Convert functions into methods where
appropriate.  Split large methods into smaller more flexible ones.
Inherit methods from other modules if appropriate.

Avoid class name tests like: C<die "Invalid" unless ref $ref eq 'FOO'>.
Generally you can delete the C<eq 'FOO'> part with no harm at all.
Let the objects look after themselves! Generally, avoid hard-wired
class names as far as possible.

Avoid C<< $r->Class::func() >> where using C<@ISA=qw(... Class ...)> and
C<< $r->func() >> would work.

Use autosplit so little used or newly added functions won't be a
burden to programs that don't use them. Add test functions to
the module after __END__ either using AutoSplit or by saying:

 eval join('',<main::DATA>) || die $@ unless caller();

Does your module pass the 'empty subclass' test? If you say
C<@SUBCLASS::ISA = qw(YOURCLASS);> your applications should be able
to use SUBCLASS in exactly the same way as YOURCLASS.  For example,
does your application still work if you change:  C<< $obj = YOURCLASS->new(); >>
into: C<< $obj = SUBCLASS->new(); >> ?

Avoid keeping any state information in your packages. It makes it
difficult for multiple other packages to use yours. Keep state
information in objects.

Always use B<-w>.

Try to C<use strict;> (or C<use strict qw(...);>).
Remember that you can add C<no strict qw(...);> to individual blocks
of code that need less strictness.

Always use B<-w>.

Follow the guidelines in L<perlstyle>.

Always use B<-w>.

=item  *

Some simple style guidelines

The perlstyle manual supplied with Perl has many helpful points.

Coding style is a matter of personal taste. Many people evolve their
style over several years as they learn what helps them write and
maintain good code.  Here's one set of assorted suggestions that
seem to be widely used by experienced developers:

Use underscores to separate words.  It is generally easier to read
$var_names_like_this than $VarNamesLikeThis, especially for
non-native speakers of English. It's also a simple rule that works
consistently with VAR_NAMES_LIKE_THIS.

Package/Module names are an exception to this rule. Perl informally
reserves lowercase module names for 'pragma' modules like integer
and strict. Other modules normally begin with a capital letter and
use mixed case with no underscores (need to be short and portable).

You may find it helpful to use letter case to indicate the scope
or nature of a variable. For example:

 $ALL_CAPS_HERE   constants only (beware clashes with Perl vars)
 $Some_Caps_Here  package-wide global/static
 $no_caps_here    function scope my() or local() variables

Function and method names seem to work best as all lowercase.
e.g., C<< $obj->as_string() >>.

You can use a leading underscore to indicate that a variable or
function should not be used outside the package that defined it.

=item  *

Select what to export.

Do NOT export method names!

Do NOT export anything else by default without a good reason!

Exports pollute the namespace of the module user.  If you must
export try to use @EXPORT_OK in preference to @EXPORT and avoid
short or common names to reduce the risk of name clashes.

Generally anything not exported is still accessible from outside the
module using the ModuleName::item_name (or C<< $blessed_ref->method >>)
syntax.  By convention you can use a leading underscore on names to
indicate informally that they are 'internal' and not for public use.

(It is actually possible to get private functions by saying:
C<my $subref = sub { ... };  &$subref;>.  But there's no way to call that
directly as a method, because a method must have a name in the symbol
table.)

As a general rule, if the module is trying to be object oriented
then export nothing. If it's just a collection of functions then
@EXPORT_OK anything but use @EXPORT with caution.

=item  *

Select a name for the module.

This name should be as descriptive, accurate, and complete as
possible.  Avoid any risk of ambiguity. Always try to use two or
more whole words.  Generally the name should reflect what is special
about what the module does rather than how it does it.  Please use
nested module names to group informally or categorize a module.
There should be a very good reason for a module not to have a nested name.
Module names should begin with a capital letter.

Having 57 modules all called Sort will not make life easy for anyone
(though having 23 called Sort::Quick is only marginally better :-).
Imagine someone trying to install your module alongside many others.

If you are developing a suite of related modules/classes it's good
practice to use nested classes with a common prefix as this will
avoid namespace clashes. For example: Xyz::Control, Xyz::View,
Xyz::Model etc. Use the modules in this list as a naming guide.

If adding a new module to a set, follow the original author's
standards for naming modules and the interface to methods in
those modules.

If developing modules for private internal or project specific use,
that will never be released to the public, then you should ensure
that their names will not clash with any future public module. You
can do this either by using the reserved Local::* category or by
using a category name that includes an underscore like Foo_Corp::*.

To be portable each component of a module name should be limited to
11 characters. If it might be used on MS-DOS then try to ensure each is
unique in the first 8 characters. Nested modules make this easier.

For additional guidance on the naming of modules, please consult:

    https://pause.perl.org/pause/query?ACTION=pause_namingmodules

or send mail to the <module-authors@perl.org> mailing list.

=item  *

Have you got it right?

How do you know that you've made the right decisions? Have you
picked an interface design that will cause problems later? Have
you picked the most appropriate name? Do you have any questions?

The best way to know for sure, and pick up many helpful suggestions,
is to ask someone who knows. The <module-authors@perl.org> mailing list
is useful for this purpose; it's also accessible via news interface as
perl.module-authors at nntp.perl.org.

All you need to do is post a short summary of the module, its
purpose and interfaces. A few lines on each of the main methods is
probably enough. (If you post the whole module it might be ignored
by busy people - generally the very people you want to read it!)

Don't worry about posting if you can't say when the module will be
ready - just say so in the message. It might be worth inviting
others to help you, they may be able to complete it for you!

=item  *

README and other Additional Files.

It's well known that software developers usually fully document the
software they write. If, however, the world is in urgent need of
your software and there is not enough time to write the full
documentation please at least provide a README file containing:

=over 10

=item *

A description of the module/package/extension etc.

=item *

A copyright notice - see below.

=item *

Prerequisites - what else you may need to have.

=item *

How to build it - possible changes to Makefile.PL etc.

=item *

How to install it.

=item *

Recent changes in this release, especially incompatibilities

=item *

Changes / enhancements you plan to make in the future.

=back

If the README file seems to be getting too large you may wish to
split out some of the sections into separate files: INSTALL,
Copying, ToDo etc.

=over 4

=item *

Adding a Copyright Notice.

How you choose to license your work is a personal decision.
The general mechanism is to assert your Copyright and then make
a declaration of how others may copy/use/modify your work.

Perl, for example, is supplied with two types of licence: The GNU GPL
and The Artistic Licence (see the files README, Copying, and Artistic,
or L<perlgpl> and L<perlartistic>).  Larry has good reasons for NOT
just using the GNU GPL.

My personal recommendation, out of respect for Larry, Perl, and the
Perl community at large is to state something simply like:

 Copyright (c) 1995 Your Name. All rights reserved.
 This program is free software; you can redistribute it and/or
 modify it under the same terms as Perl itself.

This statement should at least appear in the README file. You may
also wish to include it in a Copying file and your source files.
Remember to include the other words in addition to the Copyright.

=item  *

Give the module a version/issue/release number.

To be fully compatible with the Exporter and MakeMaker modules you
should store your module's version number in a non-my package
variable called $VERSION.  This should be a positive floating point
number with at least two digits after the decimal (i.e., hundredths,
e.g, C<$VERSION = "0.01">).  Don't use a "1.3.2" style version.
See L<Exporter> for details.

It may be handy to add a function or method to retrieve the number.
Use the number in announcements and archive file names when
releasing the module (ModuleName-1.02.tar.Z).
See perldoc ExtUtils::MakeMaker.pm for details.

=item  *

How to release and distribute a module.

If possible, register the module with CPAN. Follow the instructions
and links on:

   https://www.cpan.org/modules/04pause.html

and upload to:

   https://pause.perl.org/

and notify <modules@perl.org>. This will allow anyone to install
your module using the C<cpan> tool distributed with Perl.

By using the WWW interface you can ask the Upload Server to mirror
your modules from your ftp or WWW site into your own directory on
CPAN!

=item  *

Take care when changing a released module.

Always strive to remain compatible with previous released versions.
Otherwise try to add a mechanism to revert to the
old behavior if people rely on it.  Document incompatible changes.

=back

=back

=head2 Guidelines for Converting Perl 4 Library Scripts into Modules

=over 4

=item  *

There is no requirement to convert anything.

If it ain't broke, don't fix it! Perl 4 library scripts should
continue to work with no problems. You may need to make some minor
changes (like escaping non-array @'s in double quoted strings) but
there is no need to convert a .pl file into a Module for just that.

=item  *

Consider the implications.

All Perl applications that make use of the script will need to
be changed (slightly) if the script is converted into a module.  Is
it worth it unless you plan to make other changes at the same time?

=item  *

Make the most of the opportunity.

If you are going to convert the script to a module you can use the
opportunity to redesign the interface.  The guidelines for module
creation above include many of the issues you should consider.

=item  *

The pl2pm utility will get you started.

This utility will read *.pl files (given as parameters) and write
corresponding *.pm files. The pl2pm utilities does the following:

=over 10

=item *

Adds the standard Module prologue lines

=item *

Converts package specifiers from ' to ::

=item *

Converts die(...) to croak(...)

=item *

Several other minor changes

=back

Being a mechanical process pl2pm is not bullet proof. The converted
code will need careful checking, especially any package statements.
Don't delete the original .pl file till the new .pm one works!

=back

=head2 Guidelines for Reusing Application Code

=over 4

=item  *

Complete applications rarely belong in the Perl Module Library.

=item  *

Many applications contain some Perl code that could be reused.

Help save the world! Share your code in a form that makes it easy
to reuse.

=item  *

Break-out the reusable code into one or more separate module files.

=item  *

Take the opportunity to reconsider and redesign the interfaces.

=item  *

In some cases the 'application' can then be reduced to a small

fragment of code built on top of the reusable modules. In these cases
the application could invoked as:

     % perl -e 'use Module::Name; method(@ARGV)' ...
or
     % perl -mModule::Name ...    (in perl5.002 or higher)

=back

=head1 NOTE

Perl does not enforce private and public parts of its modules as you may
have been used to in other languages like C++, Ada, or Modula-17.  Perl
doesn't have an infatuation with enforced privacy.  It would prefer
that you stayed out of its living room because you weren't invited, not
because it has a shotgun.

The module and its user have a contract, part of which is common law,
and part of which is "written".  Part of the common law contract is
that a module doesn't pollute any namespace it wasn't asked to.  The
written contract for the module (A.K.A. documentation) may make other
provisions.  But then you know when you C<use RedefineTheWorld> that
you're redefining the world and willing to take the consequences.

=cut

read_only_bottom_close_and_rename($out);
