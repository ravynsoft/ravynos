package ExtUtils::ParseXS::Utilities;
use strict;
use warnings;
use Exporter;
use File::Spec;
use ExtUtils::ParseXS::Constants ();

our $VERSION = '3.51';

our (@ISA, @EXPORT_OK);
@ISA = qw(Exporter);
@EXPORT_OK = qw(
  standard_typemap_locations
  trim_whitespace
  C_string
  valid_proto_string
  process_typemaps
  map_type
  standard_XS_defs
  assign_func_args
  analyze_preprocessor_statements
  set_cond
  Warn
  WarnHint
  current_line_number
  blurt
  death
  check_conditional_preprocessor_statements
  escape_file_for_line_directive
  report_typemap_failure
);

=head1 NAME

ExtUtils::ParseXS::Utilities - Subroutines used with ExtUtils::ParseXS

=head1 SYNOPSIS

  use ExtUtils::ParseXS::Utilities qw(
    standard_typemap_locations
    trim_whitespace
    C_string
    valid_proto_string
    process_typemaps
    map_type
    standard_XS_defs
    assign_func_args
    analyze_preprocessor_statements
    set_cond
    Warn
    blurt
    death
    check_conditional_preprocessor_statements
    escape_file_for_line_directive
    report_typemap_failure
  );

=head1 SUBROUTINES

The following functions are not considered to be part of the public interface.
They are documented here for the benefit of future maintainers of this module.

=head2 C<standard_typemap_locations()>

=over 4

=item * Purpose

Provide a list of filepaths where F<typemap> files may be found.  The
filepaths -- relative paths to files (not just directory paths) -- appear in this list in lowest-to-highest priority.

The highest priority is to look in the current directory.  

  'typemap'

The second and third highest priorities are to look in the parent of the
current directory and a directory called F<lib/ExtUtils> underneath the parent
directory.

  '../typemap',
  '../lib/ExtUtils/typemap',

The fourth through ninth highest priorities are to look in the corresponding
grandparent, great-grandparent and great-great-grandparent directories.

  '../../typemap',
  '../../lib/ExtUtils/typemap',
  '../../../typemap',
  '../../../lib/ExtUtils/typemap',
  '../../../../typemap',
  '../../../../lib/ExtUtils/typemap',

The tenth and subsequent priorities are to look in directories named
F<ExtUtils> which are subdirectories of directories found in C<@INC> --
I<provided> a file named F<typemap> actually exists in such a directory.
Example:

  '/usr/local/lib/perl5/5.10.1/ExtUtils/typemap',

However, these filepaths appear in the list returned by
C<standard_typemap_locations()> in reverse order, I<i.e.>, lowest-to-highest.

  '/usr/local/lib/perl5/5.10.1/ExtUtils/typemap',
  '../../../../lib/ExtUtils/typemap',
  '../../../../typemap',
  '../../../lib/ExtUtils/typemap',
  '../../../typemap',
  '../../lib/ExtUtils/typemap',
  '../../typemap',
  '../lib/ExtUtils/typemap',
  '../typemap',
  'typemap'

=item * Arguments

  my @stl = standard_typemap_locations( \@INC );

Reference to C<@INC>.

=item * Return Value

Array holding list of directories to be searched for F<typemap> files.

=back

=cut

SCOPE: {
  my @tm_template;

  sub standard_typemap_locations {
    my $include_ref = shift;

    if (not @tm_template) {
      @tm_template = qw(typemap);

      my $updir = File::Spec->updir();
      foreach my $dir (
          File::Spec->catdir(($updir) x 1),
          File::Spec->catdir(($updir) x 2),
          File::Spec->catdir(($updir) x 3),
          File::Spec->catdir(($updir) x 4),
      ) {
        unshift @tm_template, File::Spec->catfile($dir, 'typemap');
        unshift @tm_template, File::Spec->catfile($dir, lib => ExtUtils => 'typemap');
      }
    }

    my @tm = @tm_template;
    foreach my $dir (@{ $include_ref}) {
      my $file = File::Spec->catfile($dir, ExtUtils => 'typemap');
      unshift @tm, $file if -e $file;
    }
    return @tm;
  }
} # end SCOPE

=head2 C<trim_whitespace()>

=over 4

=item * Purpose

Perform an in-place trimming of leading and trailing whitespace from the
first argument provided to the function.

=item * Argument

  trim_whitespace($arg);

=item * Return Value

None.  Remember:  this is an I<in-place> modification of the argument.

=back

=cut

sub trim_whitespace {
  $_[0] =~ s/^\s+|\s+$//go;
}

=head2 C<C_string()>

=over 4

=item * Purpose

Escape backslashes (C<\>) in prototype strings.

=item * Arguments

      $ProtoThisXSUB = C_string($_);

String needing escaping.

=item * Return Value

Properly escaped string.

=back

=cut

sub C_string {
  my($string) = @_;

  $string =~ s[\\][\\\\]g;
  $string;
}

=head2 C<valid_proto_string()>

=over 4

=item * Purpose

Validate prototype string.

=item * Arguments

String needing checking.

=item * Return Value

Upon success, returns the same string passed as argument.

Upon failure, returns C<0>.

=back

=cut

sub valid_proto_string {
  my ($string) = @_;

  if ( $string =~ /^$ExtUtils::ParseXS::Constants::PrototypeRegexp+$/ ) {
    return $string;
  }

  return 0;
}

=head2 C<process_typemaps()>

=over 4

=item * Purpose

Process all typemap files.

=item * Arguments

  my $typemaps_object = process_typemaps( $args{typemap}, $pwd );

List of two elements:  C<typemap> element from C<%args>; current working
directory.

=item * Return Value

Upon success, returns an L<ExtUtils::Typemaps> object.

=back

=cut

sub process_typemaps {
  my ($tmap, $pwd) = @_;

  my @tm = ref $tmap ? @{$tmap} : ($tmap);

  foreach my $typemap (@tm) {
    die "Can't find $typemap in $pwd\n" unless -r $typemap;
  }

  push @tm, standard_typemap_locations( \@INC );

  require ExtUtils::Typemaps;
  my $typemap = ExtUtils::Typemaps->new;
  foreach my $typemap_loc (@tm) {
    next unless -f $typemap_loc;
    # skip directories, binary files etc.
    warn("Warning: ignoring non-text typemap file '$typemap_loc'\n"), next
      unless -T $typemap_loc;

    $typemap->merge(file => $typemap_loc, replace => 1);
  }

  return $typemap;
}

=head2 C<map_type()>

=over 4

=item * Purpose

Performs a mapping at several places inside C<PARAGRAPH> loop.

=item * Arguments

  $type = map_type($self, $type, $varname);

List of three arguments.

=item * Return Value

String holding augmented version of second argument.

=back

=cut

sub map_type {
  my ($self, $type, $varname) = @_;

  # C++ has :: in types too so skip this
  $type =~ tr/:/_/ unless $self->{RetainCplusplusHierarchicalTypes};
  $type =~ s/^array\(([^,]*),(.*)\).*/$1 */s;
  if ($varname) {
    if ($type =~ / \( \s* \* (?= \s* \) ) /xg) {
      (substr $type, pos $type, 0) = " $varname ";
    }
    else {
      $type .= "\t$varname";
    }
  }
  return $type;
}

=head2 C<standard_XS_defs()>

=over 4

=item * Purpose

Writes to the C<.c> output file certain preprocessor directives and function
headers needed in all such files.

=item * Arguments

None.

=item * Return Value

Returns true.

=back

=cut

sub standard_XS_defs {
  print <<"EOF";
#ifndef PERL_UNUSED_VAR
#  define PERL_UNUSED_VAR(var) if (0) var = var
#endif

#ifndef dVAR
#  define dVAR		dNOOP
#endif


/* This stuff is not part of the API! You have been warned. */
#ifndef PERL_VERSION_DECIMAL
#  define PERL_VERSION_DECIMAL(r,v,s) (r*1000000 + v*1000 + s)
#endif
#ifndef PERL_DECIMAL_VERSION
#  define PERL_DECIMAL_VERSION \\
	  PERL_VERSION_DECIMAL(PERL_REVISION,PERL_VERSION,PERL_SUBVERSION)
#endif
#ifndef PERL_VERSION_GE
#  define PERL_VERSION_GE(r,v,s) \\
	  (PERL_DECIMAL_VERSION >= PERL_VERSION_DECIMAL(r,v,s))
#endif
#ifndef PERL_VERSION_LE
#  define PERL_VERSION_LE(r,v,s) \\
	  (PERL_DECIMAL_VERSION <= PERL_VERSION_DECIMAL(r,v,s))
#endif

/* XS_INTERNAL is the explicit static-linkage variant of the default
 * XS macro.
 *
 * XS_EXTERNAL is the same as XS_INTERNAL except it does not include
 * "STATIC", ie. it exports XSUB symbols. You probably don't want that
 * for anything but the BOOT XSUB.
 *
 * See XSUB.h in core!
 */


/* TODO: This might be compatible further back than 5.10.0. */
#if PERL_VERSION_GE(5, 10, 0) && PERL_VERSION_LE(5, 15, 1)
#  undef XS_EXTERNAL
#  undef XS_INTERNAL
#  if defined(__CYGWIN__) && defined(USE_DYNAMIC_LOADING)
#    define XS_EXTERNAL(name) __declspec(dllexport) XSPROTO(name)
#    define XS_INTERNAL(name) STATIC XSPROTO(name)
#  endif
#  if defined(__SYMBIAN32__)
#    define XS_EXTERNAL(name) EXPORT_C XSPROTO(name)
#    define XS_INTERNAL(name) EXPORT_C STATIC XSPROTO(name)
#  endif
#  ifndef XS_EXTERNAL
#    if defined(HASATTRIBUTE_UNUSED) && !defined(__cplusplus)
#      define XS_EXTERNAL(name) void name(pTHX_ CV* cv __attribute__unused__)
#      define XS_INTERNAL(name) STATIC void name(pTHX_ CV* cv __attribute__unused__)
#    else
#      ifdef __cplusplus
#        define XS_EXTERNAL(name) extern "C" XSPROTO(name)
#        define XS_INTERNAL(name) static XSPROTO(name)
#      else
#        define XS_EXTERNAL(name) XSPROTO(name)
#        define XS_INTERNAL(name) STATIC XSPROTO(name)
#      endif
#    endif
#  endif
#endif

/* perl >= 5.10.0 && perl <= 5.15.1 */


/* The XS_EXTERNAL macro is used for functions that must not be static
 * like the boot XSUB of a module. If perl didn't have an XS_EXTERNAL
 * macro defined, the best we can do is assume XS is the same.
 * Dito for XS_INTERNAL.
 */
#ifndef XS_EXTERNAL
#  define XS_EXTERNAL(name) XS(name)
#endif
#ifndef XS_INTERNAL
#  define XS_INTERNAL(name) XS(name)
#endif

/* Now, finally, after all this mess, we want an ExtUtils::ParseXS
 * internal macro that we're free to redefine for varying linkage due
 * to the EXPORT_XSUB_SYMBOLS XS keyword. This is internal, use
 * XS_EXTERNAL(name) or XS_INTERNAL(name) in your code if you need to!
 */

#undef XS_EUPXS
#if defined(PERL_EUPXS_ALWAYS_EXPORT)
#  define XS_EUPXS(name) XS_EXTERNAL(name)
#else
   /* default to internal */
#  define XS_EUPXS(name) XS_INTERNAL(name)
#endif

EOF

  print <<"EOF";
#ifndef PERL_ARGS_ASSERT_CROAK_XS_USAGE
#define PERL_ARGS_ASSERT_CROAK_XS_USAGE assert(cv); assert(params)

/* prototype to pass -Wmissing-prototypes */
STATIC void
S_croak_xs_usage(const CV *const cv, const char *const params);

STATIC void
S_croak_xs_usage(const CV *const cv, const char *const params)
{
    const GV *const gv = CvGV(cv);

    PERL_ARGS_ASSERT_CROAK_XS_USAGE;

    if (gv) {
        const char *const gvname = GvNAME(gv);
        const HV *const stash = GvSTASH(gv);
        const char *const hvname = stash ? HvNAME(stash) : NULL;

        if (hvname)
	    Perl_croak_nocontext("Usage: %s::%s(%s)", hvname, gvname, params);
        else
	    Perl_croak_nocontext("Usage: %s(%s)", gvname, params);
    } else {
        /* Pants. I don't think that it should be possible to get here. */
	Perl_croak_nocontext("Usage: CODE(0x%" UVxf ")(%s)", PTR2UV(cv), params);
    }
}
#undef  PERL_ARGS_ASSERT_CROAK_XS_USAGE

#define croak_xs_usage        S_croak_xs_usage

#endif

/* NOTE: the prototype of newXSproto() is different in versions of perls,
 * so we define a portable version of newXSproto()
 */
#ifdef newXS_flags
#define newXSproto_portable(name, c_impl, file, proto) newXS_flags(name, c_impl, file, proto, 0)
#else
#define newXSproto_portable(name, c_impl, file, proto) (PL_Sv=(SV*)newXS(name, c_impl, file), sv_setpv(PL_Sv, proto), (CV*)PL_Sv)
#endif /* !defined(newXS_flags) */

#if PERL_VERSION_LE(5, 21, 5)
#  define newXS_deffile(a,b) Perl_newXS(aTHX_ a,b,file)
#else
#  define newXS_deffile(a,b) Perl_newXS_deffile(aTHX_ a,b)
#endif

EOF
  return 1;
}

=head2 C<assign_func_args()>

=over 4

=item * Purpose

Perform assignment to the C<func_args> attribute.

=item * Arguments

  $string = assign_func_args($self, $argsref, $class);

List of three elements.  Second is an array reference; third is a string.

=item * Return Value

String.

=back

=cut

sub assign_func_args {
  my ($self, $argsref, $class) = @_;
  my @func_args = @{$argsref};
  shift @func_args if defined($class);

  for my $arg (@func_args) {
    $arg =~ s/^/&/ if $self->{in_out}->{$arg};
  }
  return join(", ", @func_args);
}

=head2 C<analyze_preprocessor_statements()>

=over 4

=item * Purpose

Within each function inside each Xsub, print to the F<.c> output file certain
preprocessor statements.

=item * Arguments

      ( $self, $XSS_work_idx, $BootCode_ref ) =
        analyze_preprocessor_statements(
          $self, $statement, $XSS_work_idx, $BootCode_ref
        );

List of four elements.

=item * Return Value

Modifed values of three of the arguments passed to the function.  In
particular, the C<XSStack> and C<InitFileCode> attributes are modified.

=back

=cut

sub analyze_preprocessor_statements {
  my ($self, $statement, $XSS_work_idx, $BootCode_ref) = @_;

  if ($statement eq 'if') {
    $XSS_work_idx = @{ $self->{XSStack} };
    push(@{ $self->{XSStack} }, {type => 'if'});
  }
  else {
    $self->death("Error: '$statement' with no matching 'if'")
      if $self->{XSStack}->[-1]{type} ne 'if';
    if ($self->{XSStack}->[-1]{varname}) {
      push(@{ $self->{InitFileCode} }, "#endif\n");
      push(@{ $BootCode_ref },     "#endif");
    }

    my(@fns) = keys %{$self->{XSStack}->[-1]{functions}};
    if ($statement ne 'endif') {
      # Hide the functions defined in other #if branches, and reset.
      @{$self->{XSStack}->[-1]{other_functions}}{@fns} = (1) x @fns;
      @{$self->{XSStack}->[-1]}{qw(varname functions)} = ('', {});
    }
    else {
      my($tmp) = pop(@{ $self->{XSStack} });
      0 while (--$XSS_work_idx
           && $self->{XSStack}->[$XSS_work_idx]{type} ne 'if');
      # Keep all new defined functions
      push(@fns, keys %{$tmp->{other_functions}});
      @{$self->{XSStack}->[$XSS_work_idx]{functions}}{@fns} = (1) x @fns;
    }
  }
  return ($self, $XSS_work_idx, $BootCode_ref);
}

=head2 C<set_cond()>

=over 4

=item * Purpose

=item * Arguments

=item * Return Value

=back

=cut

sub set_cond {
  my ($ellipsis, $min_args, $num_args) = @_;
  my $cond;
  if ($ellipsis) {
    $cond = ($min_args ? qq(items < $min_args) : 0);
  }
  elsif ($min_args == $num_args) {
    $cond = qq(items != $min_args);
  }
  else {
    $cond = qq(items < $min_args || items > $num_args);
  }
  return $cond;
}

=head2 C<current_line_number()>

=over 4

=item * Purpose

Figures out the current line number in the XS file.

=item * Arguments

C<$self>

=item * Return Value

The current line number.

=back

=cut

sub current_line_number {
  my $self = shift;
  my $line_number = $self->{line_no}->[@{ $self->{line_no} } - @{ $self->{line} } -1];
  return $line_number;
}

=head2 C<Warn()>

=over 4

=item * Purpose

Print warnings with line number details at the end.

=item * Arguments

List of text to output.

=item * Return Value

None.

=back

=cut

sub Warn {
  my ($self)=shift;
  $self->WarnHint(@_,undef);
}

=head2 C<WarnHint()>

=over 4

=item * Purpose

Prints warning with line number details. The last argument is assumed
to be a hint string.

=item * Arguments

List of strings to warn, followed by one argument representing a hint.
If that argument is defined then it will be split on newlines and output
line by line after the main warning.

=item * Return Value

None.

=back

=cut

sub WarnHint {
  warn _MsgHint(@_);
}

=head2 C<_MsgHint()>

=over 4

=item * Purpose

Constructs an exception message with line number details. The last argument is
assumed to be a hint string.

=item * Arguments

List of strings to warn, followed by one argument representing a hint.
If that argument is defined then it will be split on newlines and concatenated
line by line (parenthesized) after the main message.

=item * Return Value

The constructed string.

=back

=cut


sub _MsgHint {
  my $self = shift;
  my $hint = pop;
  my $warn_line_number = $self->current_line_number();
  my $ret = join("",@_) . " in $self->{filename}, line $warn_line_number\n";
  if ($hint) {
    $ret .= "    ($_)\n" for split /\n/, $hint;
  }
  return $ret;
}

=head2 C<blurt()>

=over 4

=item * Purpose

=item * Arguments

=item * Return Value

=back

=cut

sub blurt {
  my $self = shift;
  $self->Warn(@_);
  $self->{errors}++
}

=head2 C<death()>

=over 4

=item * Purpose

=item * Arguments

=item * Return Value

=back

=cut

sub death {
  my ($self) = (@_);
  my $message = _MsgHint(@_,"");
  if ($self->{die_on_error}) {
    die $message;
  } else {
    warn $message;
  }
  exit 1;
}

=head2 C<check_conditional_preprocessor_statements()>

=over 4

=item * Purpose

=item * Arguments

=item * Return Value

=back

=cut

sub check_conditional_preprocessor_statements {
  my ($self) = @_;
  my @cpp = grep(/^\#\s*(?:if|e\w+)/, @{ $self->{line} });
  if (@cpp) {
    my $cpplevel;
    for my $cpp (@cpp) {
      if ($cpp =~ /^\#\s*if/) {
        $cpplevel++;
      }
      elsif (!$cpplevel) {
        $self->Warn("Warning: #else/elif/endif without #if in this function");
        print STDERR "    (precede it with a blank line if the matching #if is outside the function)\n"
          if $self->{XSStack}->[-1]{type} eq 'if';
        return;
      }
      elsif ($cpp =~ /^\#\s*endif/) {
        $cpplevel--;
      }
    }
    $self->Warn("Warning: #if without #endif in this function") if $cpplevel;
  }
}

=head2 C<escape_file_for_line_directive()>

=over 4

=item * Purpose

Escapes a given code source name (typically a file name but can also
be a command that was read from) so that double-quotes and backslashes are escaped.

=item * Arguments

A string.

=item * Return Value

A string with escapes for double-quotes and backslashes.

=back

=cut

sub escape_file_for_line_directive {
  my $string = shift;
  $string =~ s/\\/\\\\/g;
  $string =~ s/"/\\"/g;
  return $string;
}

=head2 C<report_typemap_failure>

=over 4

=item * Purpose

Do error reporting for missing typemaps.

=item * Arguments

The C<ExtUtils::ParseXS> object.

An C<ExtUtils::Typemaps> object.

The string that represents the C type that was not found in the typemap.

Optionally, the string C<death> or C<blurt> to choose
whether the error is immediately fatal or not. Default: C<blurt>

=item * Return Value

Returns nothing. Depending on the arguments, this
may call C<death> or C<blurt>, the former of which is
fatal.

=back

=cut

sub report_typemap_failure {
  my ($self, $tm, $ctype, $error_method) = @_;
  $error_method ||= 'blurt';

  my @avail_ctypes = $tm->list_mapped_ctypes;

  my $err = "Could not find a typemap for C type '$ctype'.\n"
            . "The following C types are mapped by the current typemap:\n'"
            . join("', '", @avail_ctypes) . "'\n";

  $self->$error_method($err);
  return();
}

1;

# vim: ts=2 sw=2 et:
