package OS2::DLL;

our $VERSION = '1.07';

use Carp;
use XSLoader;

@libs = split(/;/, $ENV{'PERL5REXX'} || $ENV{'PERLREXX'} || $ENV{'LIBPATH'} || $ENV{'PATH'});
%dlls = ();

# Preloaded methods go here.  Autoload methods go after __END__, and are
# processed by the autosplit program.

# Cannot be autoload, the autoloader is used for the REXX functions.

my $load_with_dirs = sub {
	my ($class, $file, @where) = (@_);
	return $dlls{$file} if $dlls{$file};
	my $handle;
	foreach (@where) {
		$handle = DynaLoader::dl_load_file("$_/$file.dll");
		last if $handle;
	}
	$handle = DynaLoader::dl_load_file($file) unless $handle;
	return undef unless $handle;
	my @packs = $INC{'OS2/REXX.pm'} ? qw(OS2::DLL::dll OS2::REXX) : 'OS2::DLL::dll';
	my $p = "OS2::DLL::dll::$file";
	@{"$p\::ISA"} = @packs;
	*{"$p\::AUTOLOAD"} = \&OS2::DLL::dll::AUTOLOAD;
	return $dlls{$file} = 
	  bless {Handle => $handle, File => $file, Queue => 'SESSION' }, $p;
};

my $new_dll = sub {
  my ($dirs, $class, $file) = (shift, shift, shift);
  my $handle;
  push @_, @libs if $dirs;
  $handle = $load_with_dirs->($class, $file, @_)
    and return $handle;
  my $path = @_ ? " from '@_'" : '';
  my $err = DynaLoader::dl_error();
  $err =~ s/\s+at\s+\S+\s+line\s+\S+\s*\z//;
  croak "Can't load '$file'$path: $err";
};

sub new {
  confess 'Usage: OS2::DLL->new( <file> [<dirs>] )' unless @_ >= 2;
  $new_dll->(1, @_);
}

sub module {
  confess 'Usage: OS2::DLL->module( <file> [<dirs>] )' unless @_ >= 2;
  $new_dll->(0, @_);
}

sub load {
  confess 'Usage: load OS2::DLL <file> [<dirs>]' unless $#_ >= 1;
  $load_with_dirs->(@_, @libs);
}

sub libPath_find {
  my ($name, $flags, @path) = (shift, shift);
  $flags = 0x7 unless defined $flags;
  push @path, split /;/, OS2::extLibpath	if $flags & 0x1;	# BEGIN
  push @path, split /;/, OS2::libPath		if $flags & 0x2;
  push @path, split /;/, OS2::extLibpath(1)	if $flags & 0x4;	# END
  s,(?![/\\])$,/,  for @path;
  s,\\,/,g	   for @path;
  $name .= ".dll" unless $name =~ /\.[^\\\/]*$/;
  $_ .= $name for @path;
  return grep -f $_, @path if $flags & 0x8;
  -f $_ and return $_ for @path;
  return;
}

package OS2::DLL::dll;
use Carp;
@ISA = 'OS2::DLL';

sub AUTOLOAD {
    $AUTOLOAD =~ /^OS2::DLL::dll::.+::(.+)$/
      or confess("Undefined subroutine &$AUTOLOAD called");
    return undef if $1 eq "DESTROY";
    die "AUTOLOAD loop" if $1 eq "AUTOLOAD";
    $_[0]->find($1) or confess($@);
    goto &$AUTOLOAD;
}

sub wrapper_REXX {
	confess 'Usage: $dllhandle->wrapper_REXX($func_name)' unless @_ == 2;
	my $self   = shift;
	my $file   = $self->{File};
	my $handle = $self->{Handle};
	my $prefix = exists($self->{Prefix}) ? $self->{Prefix} : "";
	my $queue  = $self->{Queue};
	my $name = shift;
	$prefix = '' if $name =~ /^#\d+/;	# loading by ordinal
	my $addr = (DynaLoader::dl_find_symbol($handle, uc $prefix.$name)
		    || DynaLoader::dl_find_symbol($handle, $prefix.$name));
	return sub {
	  OS2::DLL::_call($name, $addr, $queue, @_);
	} if $addr;
	my $err = DynaLoader::dl_error();
	$err =~ s/\s+at\s+\S+\s+line\s+\S+\s*\z//;
	croak "Can't find symbol `$name' in DLL `$file': $err";
}

sub find
{
	my $self   = shift;
	my $file   = $self->{File};
	my $p	   = ref $self;
	foreach (@_) {
		my $f = eval {$self->wrapper_REXX($_)} or return 0;
		${"${p}::"}{$_} = sub { shift; $f->(@_) };
	}
	return 1;
}

sub handle	{ shift->{Handle} }
sub fullname	{ OS2::DLLname(0x202, shift->handle) }
#sub modname	{ OS2::DLLname(0x201, shift->handle) }

sub has_f32 {
   my $handle = shift->handle;
   my $name = shift;
   DynaLoader::dl_find_symbol($handle, $name);
}

XSLoader::load 'OS2::DLL';

1;
__END__

=head1 NAME

OS2::DLL - access to DLLs with REXX calling convention.

=head2 NOTE

When you use this module, the REXX variable pool is not available.

See documentation of L<OS2::REXX> module if you need the variable pool.

=head1 SYNOPSIS

 use OS2::DLL;
 $emx_dll = OS2::DLL->module('emx');
 $emx_version = $emx_dll->emx_revision();
 $func_emx_version = $emx_dll->wrapper_REXX('#128'); # emx_revision
 $emx_version = $func_emx_version->();

=head1 DESCRIPTION

=head2 Create a DLL handle

	$dll = OS2::DLL->module( NAME [, WHERE] );

Loads an OS/2 module NAME, looking in directories WHERE (adding the
extension F<.dll>), if the DLL is not found there, loads in the usual OS/2 way
(via LIBPATH and other settings).  Croaks with a verbose report on failure.

The DLL is not unloaded when the return value is destroyed.

=head2 Create a DLL handle (looking in some strange locations)

	$dll = OS2::DLL->new( NAME [, WHERE] );

Same as C<module>|L<Create a DLL handle>, but in addition to WHERE, looks
in environment paths PERL5REXX, PERLREXX, PATH (provided for backward
compatibility).

=head2 Loads DLL by name

	$dll = load OS2::DLL NAME [, WHERE];

Same as C<new>|L<Create a DLL handle (looking in some strange locations)>,
but returns DLL object reference, or undef on failure (in this case one can
get the reason via C<DynaLoader::dl_error()>) (provided for backward
compatibility).

=head2 Check for functions (optional):

	BOOL = $dll->find(NAME [, NAME [, ...]]);

Returns true if all functions are available.  As a side effect, creates
a REXX wrapper with the specified name in the package constructed by the name
of the DLL so that the next call to C<< $dll->NAME() >> will pick up the cached
method.

=head2 Create a Perl wrapper (optional):

	$func = $dll->wrapper_REXX(NAME);

Returns a reference to a Perl function wrapper for the entry point NAME
in the DLL.  Similar to the OS/2 API, the NAME may be C<"#123"> - in this case
the ordinal is loaded.   Croaks with a meaningful error message if NAME does
not exists (although the message for the case when the name is an ordinal may
be confusing).

=head2 Call external function with REXX calling convention:

	$ret_string = $dll->function_name(arguments);

Returns the return string if the REXX return code is 0, else undef.
Dies with error message if the function is not available.  On the first call
resolves the name in the DLL and caches the Perl wrapper; future calls go
through the wrapper.

Unless used inside REXX environment (see L<OS2::REXX>), the REXX runtime
environment (variable pool, queue etc.) is not available to the called
function.

=head1 Inspecting the module

=over

=item $module->handle

=item $module->fullname

Return the (integer) handle and full path name of a loaded DLL.

TODO: the module name (whatever is specified in the C<LIBRARY> statement
of F<.def> file when linking) via OS2::Proc.

=item $module->has_f32($name)

Returns the address of a 32-bit entry point with name $name, or 0 if none
found.  (Keep in mind that some entry points may be 16-bit, and some may have
capitalized names comparing to callable-from-C counterparts.)  Name of the
form C<#197> will find entry point with ordinal 197.

=item libPath_find($name [, $flags])

Looks for the DLL $name on C<BEGINLIBPATH>, C<LIBPATH>, C<ENDLIBPATH> if
bits 0x1, 0x2, 0x4 of $flags are set correspondingly.  If called with no
arguments, looks on all 3 locations.  Returns the full name of the found
file.  B<DLL is not loaded.>

$name has F<.dll> appended unless it already has an extension.

=back

=head1 Low-level API

=over

=item Call a _System linkage function via a pointer

If a function takes up to 20 ULONGs and returns ULONG:

 $res = call20( $pointer, $arg0, $arg1, ...);

=item Same for packed arguments:

 $res = call20_p( $pointer, pack 'L20', $arg0, $arg1, ...);

=item Same for C<regparm(3)> function:

 $res = call20_rp3( $pointer, $arg0, $arg1, ...);

=item Same for packed arguments and C<regparm(3)> function

 $res = call20_rp3_p( $pointer, pack 'L20', $arg0, $arg1, ...);

=item Same for a function which returns non-0 and sets system-error on error

 call20_Dos( $msg, $pointer, $arg0, $arg1, ...); # die("$msg: $^E")
                                                            if error

[Good for C<Dos*> API - and rare C<Win*> calls.]

=item Same for a function which returns 0 and sets WinLastError() on error

 $res = call20_Win( $msg, $pointer, $arg0, $arg1, ...);
 # would die("$msg: $^E") if error

[Good for most of C<Win*> API.]

=item Same for a function which returns 0 and sets WinLastError() on error but
0 is also a valid return

 $res = call20_Win_0OK( $msg, $pointer, $arg0, $arg1, ...);
 # would die("$msg: $^E") if error

[Good for some of C<Win*> API.]

=item As previous, but without die()

 $res = call20_Win_0OK_survive( $pointer, $arg0, $arg1, ...);
 if ($res == 0 and $^E) {	# Do error processing here
 }

[Good for some of C<Win*> API.]

=back

=head1 ENVIRONMENT

If C<PERL_REXX_DEBUG> is set, emits debugging output.  Looks for DLLs
in C<PERL5REXX>, C<PERLREXX>, C<PATH>.

=head1 AUTHOR

Extracted by Ilya Zakharevich perl-module-OS2-DLL@ilyaz.org from L<OS2::REXX>
written by Andreas Kaiser ak@ananke.s.bawue.de.

=cut
