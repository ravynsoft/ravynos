################################################################################
#
#  PPPort_pm.PL -- generate PPPort.pm
#
# Set the environment variable DPPP_CHECK_LEVEL to more than zero for some
# extra checking. 1 or 2 currently

################################################################################
#
#  Version 3.x, Copyright (C) 2004-2013, Marcus Holland-Moritz.
#               Copyright (C) 2018, The perl5 porters
#  Version 2.x, Copyright (C) 2001, Paul Marquess.
#  Version 1.x, Copyright (C) 1999, Kenneth Albanowski.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#
################################################################################

use strict;
BEGIN { $^W = 1; }
require "./parts/ppptools.pl";
require "./parts/inc/inctools";

my $INCLUDE = 'parts/inc';
my $DPPP = 'DPPP_';

# The keys of %embed are the names of the items found in all the .fnc files,
# and each value is all the information parse_embed returns for that item.
my %embed = map { ( $_->{name} => $_ ) }
            parse_embed(qw(parts/embed.fnc parts/apidoc.fnc parts/ppport.fnc));

my(%provides, %prototypes, %explicit);

my $data = do { local $/; <DATA> };

# Call include(file, params) for every line that begins with %include
# These fill in %provides and %prototypes.
# The keys of %provides are the items provided by Devel::PPPort, and each
# value is the name of the file (in parts/inc/) that has the code to provide
# it.
# An entry in %prototypes looks like:
#   'grok_bin' => 'UV grok_bin(pTHX_ const char * start, STRLEN * len_p, I32 * flags, NV * result)',

$data =~ s{^\%(include)\s+(\w+)((?:[^\S\r\n]+.*?)?)\s*$}
          {eval "$1('$2', $3)" or die $@}gem;

# And expand it.
$data = expand($data);

# Just the list of provided items.
my @provided = sort dictionary_order keys %provides;

# which further expands $data.
$data =~ s{^(.*)__PROVIDED_API__(\s*?)^}
          {join '', map "$1$_\n", @provided}gem;

{
  my $len = 0;
  for (keys %explicit) {
    length > $len and $len = length;
  }
  my $format = sprintf '%%-%ds  %%-%ds  %%s', $len+2, $len+5;
  $len = 3*$len + 23;

$data =~ s!^(.*)__EXPLICIT_API__(\s*?)^!
           sprintf("$1$format\n", 'Function / Variable', 'Static Request', 'Global Request') .
           $1 . '-'x$len . "\n" .
           join('', map { sprintf "$1$format\n", $explicit{$_} eq 'var' ? $_ : "$_()", "NEED_$_", "NEED_${_}_GLOBAL" }
                    sort dictionary_order keys %explicit)
          !gem;
}

# These hashes look like:
#   { ...  'gv_check' => '5.003007',
#          'gv_const_sv' => '5.009003',
#          'gv_dump' => '5.006000',
#     ... },

# What's provided when without ppport.h, as far as we've been able to
# determine
my %raw_base = %{&parse_todo('parts/base')};

# What's provided when using ppport.h, as far as we've been able to
# determine
my %raw_todo = %{&parse_todo('parts/todo')};

# The items defined by config.h are found instead by scanprov which currently
# doesn't write them to the todo files.  Instead add them here.  All such have
# the K code, and we know certain things about them, so set their flags
# accordingly.
for (keys %raw_base) {
    if (   exists $raw_base{$_}{code}
        && $raw_base{$_}{code} eq 'K')
    {
        $embed{$_}->{flags}{'A'} = 1;
        $embed{$_}->{flags}{'m'} = 1;
        $embed{$_}->{flags}{'d'} = 1;
        $embed{$_}->{flags}{'T'} = 1;
        #
        # Don't override any existing
        $raw_todo{$_} = $raw_base{$_} unless exists $raw_todo{$_};
    }
}

# Invert so each key is the 7 digit version number, and its value is an array
# of all symbols within it, like:
#          '5005003' => [
#                         'POPpx',
#                         'get_vtbl',
#                         'save_generic_svref'
#                       ],
my %todo;
for (keys %raw_todo) {
  push @{$todo{int_parse_version($raw_todo{$_}{version})}}, $_;
}

# Most recent first
my @todo_list = reverse sort keys %todo;

# Here, @todo_list contains the integer version numbers that have support.
# The first and final elements give the extremes of the supported versions.
# (Use defaults that were reasonable at the time of this commit if the
# directories are empty (which should only happen during regeneration of the
# base and todo files).).  Actually the final element is for blead (at the
# time things were regenerated), which is 1 beyond the max version supported.
my $INT_MAX_PERL = (@todo_list) ? $todo_list[0] - 1 : '5034000'; # used for __MAX_PERL__
my $MAX_PERL = format_version($INT_MAX_PERL);
my $INT_MIN_PERL = (@todo_list) ? $todo_list[-1] : 5003007;
my $MIN_PERL = format_version($INT_MIN_PERL);

# Get rid of blead.  It contains the things marked as todo, meaning they
# don't compile at all, and not getting rid of it would mean they would be
# listed as working but introduced in blead.
shift @todo_list if @todo_list && $todo_list[0] > $INT_MAX_PERL;

# check consistency between our list of everything provided, and our lists of
# what got provided when
for (@provided) {
  if (   exists $raw_todo{$_}
      && $raw_todo{$_}{version} > $INT_MIN_PERL # INT_MIN_PERL contents are real
                                                # symbols, not something to do
      && $raw_todo{$_}{version} <= $INT_MAX_PERL # Above this would be things that
                                                 # don't compile in blead
      && exists $raw_base{$_})
  {
    if ($raw_base{$_}{version} == $raw_todo{$_}{version}) {
      warn "$INCLUDE/$provides{$_} provides $_, which is still marked "
           . "todo for " . format_version($raw_todo{$_}) . "\n";
    }
    else {
      check(2, "$_ was ported back to " . format_version($raw_todo{$_}{version})
            .  " (baseline revision: "  . format_version($raw_base{$_}{version})
            . ").");
    }
  }
}

my %perl_api;
for (@provided) {
  next if /^Perl_(.*)/ && exists $embed{$1};
  next if exists $embed{$_};
  $perl_api{$_} = 1;
  check(2, "No API definition for provided element $_ found.");
}

# At this point the keys of %perl_api give the list of things we provide that
# weren't found in the .fnc files.
my @prototype_unknown = keys %perl_api;

# Add in the .fnc file definitions.
for (keys %embed) {
    $perl_api{$_} = 1;
}

# Finally, scanprov has found macros of various types
my @scanprov_found_macros = grep { $raw_base{$_}{code} =~ /[KMZ]/ } keys %raw_base;
for (@scanprov_found_macros) {
    $perl_api{$_} = 1;
}

my @perl_api = sort dictionary_order keys %perl_api;

for (@perl_api) {   # $_ is the item name
  if (exists $provides{$_} && !exists $raw_base{$_}) {
    check(2, "Mmmh, $_ doesn't seem to need backporting.");
  }

  # Create the lines that ppport.h reads.  These look like
  #     CopyD|5.009002|5.003007|p
  my $line = "$_|";
  $line .= $raw_base{$_}{version} if exists $raw_base{$_}
                # If is above the max, it means it never actually got defined
            && int_parse_version($raw_base{$_}{version}) <= $INT_MAX_PERL;
  $line .= '|';
  $line .= $raw_todo{$_}{version}
                if exists $raw_todo{$_}
                && int_parse_version($raw_todo{$_}{version}) <= $INT_MAX_PERL;
  $line .= '|';
  $line .= 'p' if exists $provides{$_};
  my $e;
  $e = $embed{$_} if exists $embed{$_};
  my $is_documented = 0;
  my $is_accessible = 0;
  my $has_thread = 1;
  if (defined $e) {
    if (exists $e->{flags}{'p'}) {    # Has 'Perl_' prefix
      my $args = $e->{args};
      $line .= 'v' if @$args && $args->[-1][0] eq '...';
    }
    $line .= 'o' if exists $e->{ppport_fnc};
    $line .= 'd' if exists $e->{flags}{'D'};  # deprecated
    $line .= 'x' if exists $e->{flags}{'x'};  # experimental
    $line .= 'c' if        exists $e->{flags}{'C'}      # core-only
                   || (    exists $e->{flags}{'X'}
                       && (exists $e->{flags}{'E'} || ! exists $e->{flags}{'m'}));
    $has_thread = 0 if exists $e->{flags}{'T'};  # No thread context parameter
    $is_accessible = 1 if exists $e->{flags}{'A'}
                       || exists $e->{flags}{'C'}
                       || (     exists $e->{flags}{'X'}
                           && ! exists $e->{flags}{'E'}
                           &&   exists $e->{flags}{'m'});
    $is_documented = 1 if exists $e->{flags}{'d'};
  }

  # scanprov adds several flags:
  #     F is for functions we didn't find in testing (usually because they are
  #       hidden behind ifdefs, like USE_THREADS if built on unthreaded perls,
  #       or vice-versa.
  #     K for config.h #defines.  These are considered provided and are
  #       documented in config.h (mnemonic: K is like c in config)
  #     M for provided, undocumented macros.  If they were documented, they
  #       would have been found before scanprov was run.
  #     Z for macros that aren't documented or provided.  (mnemonic:
  #       'Zecret').  Some of these may be hidden behind an #ifdef PERL_CORE.
  # None of these were verified by compilation
  if (exists $raw_base{$_}{code}) {
    $line .= 'V' if $raw_base{$_}{code} =~ /[FKMXZ]/;
    $is_accessible = 1 if $raw_base{$_}{code} =~ /M/;
  }
  $line .= 'n' unless $has_thread;
  $line .= 'i' unless $is_accessible;
  $line .= 'u' unless $is_documented;

  $_ = $line;
}

$data =~ s/^([\t ]*)__ALL_ELEMENTS__(\s*?)$/
           join "\n", map "$1$_", sort dictionary_order @perl_api
          /gem;

my $undocumented = "(undocumented)";

my @todo;
for (@todo_list) {
  my $ver = format_version($_);
  $ver .= " (or maybe earlier)" if $_ == $todo_list[-1];
  my $todo = "=item perl $ver\n\n";
  for (sort dictionary_order @{$todo{$_}}) {
    $todo .= "  $_";
    $todo .= "  (DEPRECATED)" if  $embed{$_}->{flags}{'D'};
    $todo .= "  (marked experimental)" if $embed{$_}->{flags}{'x'};
    $todo .= "  $undocumented" unless $embed{$_}->{flags}{'d'};
    $todo .= "\n";
  }
  push @todo, $todo;
}

if (@prototype_unknown) {
    my $todo = "=item Backported version unknown\n\n";
    for (sort dictionary_order @prototype_unknown) {
        $todo .= " $_  $undocumented\n";
    }
    push @todo, $todo;
}

$data =~ s{^__UNSUPPORTED_API__(\s*?)^}
          {join "\n", @todo}gem;

$data =~ s{__MIN_PERL__}{$MIN_PERL}g;
$data =~ s{__MAX_PERL__}{$MAX_PERL}g;

open FH, ">PPPort.pm" or die "PPPort.pm: $!\n";
print FH $data;
close FH;

exit 0;

sub include
{
  my($file, $opt) = @_;

  print "including $file\n";

  my $data = parse_partspec("$INCLUDE/$file");

  for (@{$data->{provides}}) {
    if (exists $provides{$_}) {
      if ($provides{$_} ne $file) {
        warn "$file: $_ already provided by $provides{$_}\n";
      }
    }
    else {
      $provides{$_} = $file;
    }
  }

  for (keys %{$data->{prototypes}}) {
    $prototypes{$_} = $data->{prototypes}{$_};
    $prototypes{$_} = normalize_prototype($data->{prototypes}{$_});
    $data->{implementation} =~ s/^$_(?=\s*\()/$DPPP(my_$_)/mg;
  }

  my $out = $data->{implementation};

  if (exists $opt->{indent}) {
    $out =~ s/^/$opt->{indent}/gm;
  }

  return $out;
}

sub expand
{
  my $code = shift;
  $code =~ s{^(\s*#\s*(?:el)?if\s+)(.*)$}{$1.expand_pp_expressions($2)}gem;
  $code =~ s{^\s*
              __(?:UNDEFINED|UNDEF_NOT_PROVIDED)__
              \s+
              (
                ( \w+ )
                (?: \( [^)]* \) )?
              )
              [^\r\n\S]*
              (
                (?:[^\r\n\\]|\\[^\r\n])*
                (?:
                  \\
                  (?:\r\n|[\r\n])
                  (?:[^\r\n\\]|\\[^\r\n])*
                )*
              )
            \s*$}
            {expand_undefined($2, $1, $3)}gemx;
  $code =~ s{^([^\S\r\n]*)__NEED_VAR__\s+(.*?)\s+(\w+)(?:\s*=\s*([^;]+?))?\s*;\s*$}
            {expand_need_var($1, $3, $2, $4)}gem;
  $code =~ s{^([^\S\r\n]*)__NEED_DUMMY_VAR__\s+(.*?)\s+(\w+)(?:\s*=\s*([^;]+?))?\s*;\s*$}
            {expand_need_dummy_var($1, $3, $2, $4)}gem;
  return $code;
}

sub expand_need_var
{
  my($indent, $var, $type, $init) = @_;

  $explicit{$var} = 'var';

  my $myvar = "$DPPP(my_$var)";
  $init = defined $init ? " = $init" : "";

  my $code = <<ENDCODE;
#if defined(NEED_$var)
static $type $myvar$init;
#elif defined(NEED_${var}_GLOBAL)
$type $myvar$init;
#else
extern $type $myvar;
#endif
#define $var $myvar
ENDCODE

  $code =~ s/^/$indent/mg;

  return $code;
}

sub expand_need_dummy_var
{
  my($indent, $var, $type, $init) = @_;

  $explicit{$var} = 'var';

  my $myvar = "$DPPP(dummy_$var)";
  $init = defined $init ? " = $init" : "";

  my $code = <<ENDCODE;
#if defined(NEED_$var)
static $type $myvar$init;
#elif defined(NEED_${var}_GLOBAL)
$type $myvar$init;
#else
extern $type $myvar;
#endif
ENDCODE

  $code =~ s/^/$indent/mg;

  return $code;
}

sub expand_undefined
{
  my($macro, $withargs, $def) = @_;
  my $rv = "#ifndef $macro\n#  define ";

  if (defined $def && $def =~ /\S/) {
    $rv .= sprintf "%-30s %s", $withargs, $def;
  }
  else {
    $rv .= $withargs;
  }

  $rv .= "\n#endif\n";

  return $rv;
}

sub expand_pp_expressions
{
  my $pp = shift;
  $pp =~ s/\{([^\}]+)\}/expand_pp_expr($1)/ge;
  return $pp;
}

sub expand_pp_expr
{
  my $expr = shift;

  if ($expr =~ /^\s*need\s+(\w+)\s*$/i) {
    my $func = $1;
    my $e = $embed{$func} or die "unknown API function '$func' in NEED\n";
    my $proto = make_prototype($e);
    if (exists $prototypes{$func}) {
      if (compare_prototypes($proto, $prototypes{$func})) {
        my $proto_no_pTHX = $proto;
        $proto_no_pTHX =~ s/pTHX_\s*//;
        if (compare_prototypes($proto_no_pTHX, $prototypes{$func})) {
            check(1, "differing prototypes for $func:\n  API: $proto\n  PPP: $prototypes{$func}");
        }
        else {
            check(1, "prototypes differ in pTHX_ for $func:\n  API: $proto\n  PPP: $prototypes{$func}");
        }
        $proto = $prototypes{$func};
      }
    }
    else {
      warn "found no prototype for $func\n";;
    }

    $explicit{$func} = 'func';

    $proto =~ s/\b$func(?=\s*\()/$DPPP(my_$func)/;
    my $embed = make_embed($e);

    return "defined(NEED_$func)\n"
         . "static $proto;\n"
         . "static\n"
         . "#else\n"
         . "extern $proto;\n"
         . "#endif\n"
         . "\n"
         . "#if defined(NEED_$func) || defined(NEED_${func}_GLOBAL)\n"
         . "\n"
         . "$embed\n";
  }

  die "cannot expand preprocessor expression '$expr'\n";
}

sub make_embed
{
  my $f = shift;
  my $n = $f->{name};
  my $a = do { my $x = 'a'; join ',', map { $x++ } 1 .. @{$f->{args}} };
  my $lastarg = ${$f->{args}}[-1];

  if ($f->{flags}{'T'}) {
    if ($f->{flags}{'p'}) {
      return "#define $n $DPPP(my_$n)\n" .
             "#define Perl_$n $DPPP(my_$n)";
    }
    else {
      return "#define $n $DPPP(my_$n)";
    }
  }
  else {
    my $undef = <<UNDEF;
#ifdef $n
#  undef $n
#endif
UNDEF
    if ($f->{flags}{'p'}) {
      if ($f->{flags}{'f'}) {
        return "#define Perl_$n $DPPP(my_$n)";
      }
      elsif (@$lastarg && $lastarg->[0] =~ /\.\.\./) {
        return $undef . "#define $n $DPPP(my_$n)\n" .
                        "#define Perl_$n $DPPP(my_$n)";
      }
      else {
        return $undef . "#define $n($a) $DPPP(my_$n)(aTHX_ $a)\n" .
                        "#define Perl_$n $DPPP(my_$n)";
      }
    }
    else {
      return $undef . "#define $n($a) $DPPP(my_$n)(aTHX_ $a)";
    }
  }
}

sub check
{
  my $level = shift;

  if (exists $ENV{DPPP_CHECK_LEVEL} and $ENV{DPPP_CHECK_LEVEL} >= $level) {
    print STDERR @_, "\n";
  }
}

__DATA__
################################################################################
#
#  !!!!! Do NOT edit this file directly! -- Edit PPPort_pm.PL instead. !!!!!
#
#  This file was automatically generated from the definition files in the
#  parts/inc/ subdirectory by PPPort_pm.PL. To learn more about how all this
#  works, please read the L<HACKERS> file that came with this distribution.
#
################################################################################
#
#  Perl/Pollution/Portability
#
################################################################################
#
#  Version 3.x, Copyright (C) 2004-2013, Marcus Holland-Moritz.
#               Copyright (C) 2018, The perl5 porters
#  Version 2.x, Copyright (C) 2001, Paul Marquess.
#  Version 1.x, Copyright (C) 1999, Kenneth Albanowski.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#
################################################################################

=head1 NAME

Devel::PPPort - Perl/Pollution/Portability

=head1 SYNOPSIS

  Devel::PPPort::WriteFile();   # defaults to ./ppport.h
  Devel::PPPort::WriteFile('someheader.h');

  # Same as above but retrieve contents rather than write file
  my $contents = Devel::PPPort::GetFileContents();
  my $contents = Devel::PPPort::GetFileContents('someheader.h');

=head1 Start using Devel::PPPort for XS projects

  $ cpan Devel::PPPort
  $ perl -MDevel::PPPort -e'Devel::PPPort::WriteFile'
  $ perl ppport.h --compat-version=5.6.1 --patch=diff.patch *.xs
  $ patch -p0 < diff.patch
  $ echo ppport.h >>MANIFEST

=head1 DESCRIPTION

Perl's API has changed over time, gaining new features, new functions,
increasing its flexibility, and reducing the impact on the C namespace
environment (reduced pollution). The header file written by this module,
typically F<ppport.h>, attempts to bring some of the newer Perl API
features to older versions of Perl, so that you can worry less about
keeping track of old releases, but users can still reap the benefit.

C<Devel::PPPort> contains two functions, C<WriteFile> and C<GetFileContents>.
C<WriteFile>'s only purpose is to write the F<ppport.h> C header file.
This file contains a series of macros and, if explicitly requested, functions
that allow XS modules to be built using older versions of Perl. Currently,
Perl versions from __MIN_PERL__ to __MAX_PERL__ are supported.

C<GetFileContents> can be used to retrieve the file contents rather than
writing it out.

This module is used by C<h2xs> to write the file F<ppport.h>.

=head2 Why use ppport.h?

You should use F<ppport.h> in modern code so that your code will work
with the widest range of Perl interpreters possible, without significant
additional work.

You should attempt to get older code to fully use F<ppport.h>, because the
reduced pollution of newer Perl versions is an important thing. It's so
important that the old polluting ways of original Perl modules will not be
supported very far into the future, and your module will almost certainly
break! By adapting to it now, you'll gain compatibility and a sense of
having done the electronic ecology some good.

=head2 How to use ppport.h

Don't direct the users of your module to download C<Devel::PPPort>.
They are most probably not XS writers. Also, don't make F<ppport.h>
optional. Rather, just take the most recent copy of F<ppport.h> that
you can find (e.g. by generating it with the latest C<Devel::PPPort>
release from CPAN), copy it into your project, adjust your project to
use it, test it, and distribute the header along with your module.

It is important to use the most recent version of F<ppport.h>.  You do need to
test before shipping a newer version than you already had.  One possible
failure is that someone had to convert a backported element from a macro into
a function, and actual functions must be enabled with a NEED macro to minimize
the possibility of namespace pollution.  See L<HACKERS> for details.  The
developers of C<Devel::PPPort> want to hear if there are other problems that
arise from using a later F<ppport.h>.  Use
L<https://github.com/Dual-Life/Devel-PPPort/issues> to report any.

=head2 Running ppport.h

But F<ppport.h> is more than just a C header. It's also a Perl script
that can check your source code. It will suggest hints and portability
notes, and can even make suggestions on how to change your code. You
can run it like any other Perl program:

    perl ppport.h [options] [files]

It also has embedded documentation, so you can use

    perldoc ppport.h

to find out more about how to use it.

=head1 FUNCTIONS

=head2 WriteFile

C<WriteFile> takes one optional argument. When called with one
argument, it expects to be passed a filename. When called with
no arguments, it defaults to the filename F<ppport.h>.

The function returns a true value if the file was written successfully.
Otherwise it returns a false value.

=head2 GetFileContents

C<GetFileContents> behaves like C<WriteFile> above, but returns the contents
of the would-be file rather than writing it out.

=head1 COMPATIBILITY

F<ppport.h> supports Perl versions from __MIN_PERL__ to __MAX_PERL__
in threaded and non-threaded configurations.

=head2 Provided Perl compatibility API

The header file written by this module, typically F<ppport.h>, provides access
to the following elements of the Perl API that are not otherwise available in
Perl releases older than when the elements were first introduced.  (Note that
many of these are not supported all the way back to __MIN_PERL__, but it may
be that they are supported back as far as you need; see L</Supported Perl API,
sorted by version> for that information.)

    __PROVIDED_API__

=head2 Supported Perl API, sorted by version

The table in this section lists all the Perl API elements available, sorted by
the version in which support starts.  This includes all the elements that
F<ppport.h> helps out with, as well as those elements that it doesn't.

In some cases, it doesn't make practical sense for elements to be supported
earlier than they already are.  For example, UTF-8 functionality isn't
provided prior to the release where it was first introduced.

But in other cases, it just is that no one has implemented support yet.
Patches welcome!  Some elements are ported backward for some releases, but not
all the way to __MIN_PERL__.

If an element, call it ELEMENT, is not on this list, try using this command to
find out why:

 perl ppport.h --api-info=ELEMENT

A few of the entries in the list below are marked as DEPRECATED.  You should
not use these for new code, and should be converting existing uses to use
something better.

Some of the entries in the list are marked as "experimental".  This means
these should not generally be used.  They may be removed or changed without
notice.  You can ask why they are experimental by sending email to
L<mailto:perl5-porters@perl.org>.

And some of the entries are marked as "undocumented".  This means that they
aren't necessarily considered stable, and could be changed or removed in some
future release without warning.  It is therefore a bad idea to use them
without further checking.  It could be that these are considered to be for
perl core use only; or it could be, though, that C<Devel::PPPort> doesn't know
where to find their documentation, or that it's just an oversight that they
haven't been documented.  If you want to use one, and potentially have it
backported, first send mail to L<mailto:perl5-porters@perl.org>.

=over 4

__UNSUPPORTED_API__

=back

=head1 BUGS

If you find any bugs, C<Devel::PPPort> doesn't seem to build on your
system, or any of its tests fail, please send a bug report to
L<https://github.com/Dual-Life/Devel-PPPort/issues/new>.

=head1 AUTHORS

=over 2

=item *

Version 1.x of Devel::PPPort was written by Kenneth Albanowski.

=item *

Version 2.x was ported to the Perl core by Paul Marquess.

=item *

Version 3.x was ported back to CPAN by Marcus Holland-Moritz.

=item *

Versions >= 3.22 are maintained by perl5 porters

=back

=head1 COPYRIGHT

Version 3.x, Copyright (C) 2004-2013, Marcus Holland-Moritz.

             Copyright (C) 2018-2020, The perl5 porters

Version 2.x, Copyright (C) 2001, Paul Marquess.

Version 1.x, Copyright (C) 1999, Kenneth Albanowski.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=head1 SEE ALSO

See L<h2xs>, F<ppport.h>.

=cut

package Devel::PPPort;

use strict;
use vars qw($VERSION $data);

$VERSION = '3.71';

sub _init_data
{
  $data = do { local $/; <DATA> };
  my $pkg = 'Devel::PPPort';
  $data =~ s/__PERL_VERSION__/$]/g;
  $data =~ s/__VERSION__/$VERSION/g;
  $data =~ s/__PKG__/$pkg/g;
  $data =~ s/^\|>//gm;
}

sub GetFileContents {
  my $file = shift || 'ppport.h';
  defined $data or _init_data();
  my $copy = $data;
  $copy =~ s/\bppport\.h\b/$file/g;

  return $copy;
}

sub WriteFile
{
  my $file = shift || 'ppport.h';
  my $data = GetFileContents($file);
  open F, ">$file" or return undef;
  print F $data;
  close F;

  return 1;
}

1;

__DATA__
#if 0
my $void = <<'SKIP';
#endif
/*
----------------------------------------------------------------------

    ppport.h -- Perl/Pollution/Portability Version __VERSION__

    Automatically created by __PKG__ running under perl __PERL_VERSION__.

    Do NOT edit this file directly! -- Edit PPPort_pm.PL and the
    includes in parts/inc/ instead.

    Use 'perldoc ppport.h' to view the documentation below.

----------------------------------------------------------------------

SKIP

%include ppphdoc { indent => '|>' }

%include inctools

%include ppphbin

__DATA__
*/

#ifndef _P_P_PORTABILITY_H_
#define _P_P_PORTABILITY_H_

#ifndef DPPP_NAMESPACE
#  define DPPP_NAMESPACE DPPP_
#endif

#define DPPP_CAT2(x,y) CAT2(x,y)
#define DPPP_(name) DPPP_CAT2(DPPP_NAMESPACE, name)

%include version

%include threads

%include limits

%include variables

%include subparse

%include newCONSTSUB

%include magic_defs

%include misc

%include sv_xpvf

%include SvPV

%include warn

%include format

%include uv

%include memory

%include mess

%include mPUSH

%include call

%include newRV

%include MY_CXT

%include SvREFCNT

%include newSV_type

%include newSVpv

%include Sv_set

%include shared_pv

%include HvNAME

%include gv

%include pvs

%include magic

%include cop

%include grok

%include snprintf

%include sprintf

%include exception

%include strlfuncs

%include utf8

%include pv_tools

%include locale

#endif /* _P_P_PORTABILITY_H_ */

/* End of File ppport.h */
