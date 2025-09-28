package OS2::REXX;

require Exporter;
use XSLoader;
require OS2::DLL;

@ISA = qw(Exporter);
# Items to export into callers namespace by default
# (move infrequently used names to @EXPORT_OK below)
@EXPORT = qw(REXX_call REXX_eval REXX_eval_with);
# Other items we are prepared to export if requested
@EXPORT_OK = qw(drop register);

$VERSION = '1.06';

# We cannot just put OS2::DLL in @ISA, since some scripts would use
# function interface, not method interface...

*_call = \&OS2::DLL::_call;
*load = \&OS2::DLL::load;
*find = \&OS2::DLL::find;

XSLoader::load 'OS2::REXX';

# Preloaded methods go here.  Autoload methods go after __END__, and are
# processed by the autosplit program.

sub register {_register($_) for @_}

sub prefix
{
	my $self = shift;
	$self->{Prefix} = shift;
}

sub queue
{
	my $self = shift;
	$self->{Queue} = shift;
}

sub drop
{				# Supposedly should drop anything with
                                # the given prefix. Unfortunately a
                                # loop is needed after fixpack17.
&OS2::REXX::_drop(@_);
}

sub dropall
{				# Supposedly should drop anything with
                                # the given prefix. Unfortunately a
                                # loop is needed after fixpack17.
  &OS2::REXX::_drop(@_);	# Try to drop them all.
  my $name;
  for (@_) {
    if (/\.$/) {
      OS2::REXX::_fetch('DUMMY'); # reset REXX's first/next iterator
      while (($name) = OS2::REXX::_next($_)) {
	OS2::REXX::_drop($_ . $name);
      }
    } 
  }
}

sub TIESCALAR
{
	my ($obj, $name) = @_;
	$name =~ s/^([\w!?]+)/\U$1\E/;
	return bless \$name, OS2::REXX::_SCALAR;
}	

sub TIEARRAY
{
	my ($obj, $name) = @_;
	$name =~ s/^([\w!?]+)/\U$1\E/;
	return bless [$name, 0], OS2::REXX::_ARRAY;
}

sub TIEHASH
{
	my ($obj, $name) = @_;
	$name =~ s/^([\w!?]+)/\U$1\E/;
	return bless {Stem => $name}, OS2::REXX::_HASH;
}

#############################################################################
package OS2::REXX::_SCALAR;

sub FETCH
{
	return OS2::REXX::_fetch(${$_[0]});
}

sub STORE
{
	return OS2::REXX::_set(${$_[0]}, $_[1]);
}

sub DESTROY
{
	return OS2::REXX::_drop(${$_[0]});
}

#############################################################################
package OS2::REXX::_ARRAY;

sub FETCH
{
	$_[0]->[1] = $_[1] if $_[1] > $_[0]->[1];
	return OS2::REXX::_fetch($_[0]->[0].'.'.(0+$_[1]));
}

sub STORE
{
	$_[0]->[1] = $_[1] if $_[1] > $_[0]->[1];
	return OS2::REXX::_set($_[0]->[0].'.'.(0+$_[1]), $_[2]);
}

#############################################################################
package OS2::REXX::_HASH;

require Tie::Hash;
@ISA = ('Tie::Hash');

sub FIRSTKEY
{
	my ($self) = @_;
	my $stem = $self->{Stem};

	delete $self->{List} if exists $self->{List};

	my @list = ();
	my ($name, $value);
	OS2::REXX::_fetch('DUMMY'); # reset REXX's first/next iterator
	while (($name) = OS2::REXX::_next($stem)) {
		push @list, $name;
	}
	my $key = pop @list;

	$self->{List} = \@list;
	return $key;
}

sub NEXTKEY
{
	return pop @{$_[0]->{List}};
}

sub EXISTS
{
	return defined OS2::REXX::_fetch($_[0]->{Stem}.$_[1]);
}

sub FETCH
{
	return OS2::REXX::_fetch($_[0]->{Stem}.$_[1]);
}

sub STORE
{
	return OS2::REXX::_set($_[0]->{Stem}.$_[1], $_[2]);
}

sub DELETE
{
	OS2::REXX::_drop($_[0]->{Stem}.$_[1]);
}

#############################################################################
package OS2::REXX;

1;
__END__

=head1 NAME

OS2::REXX - access to DLLs with REXX calling convention and REXX runtime.

=head2 NOTE

By default, the REXX variable pool is not available, neither
to Perl, nor to external REXX functions. To enable it, you need to put
your code inside C<REXX_call> function.  REXX functions which do not use
variables may be usable even without C<REXX_call> though.

=head1 SYNOPSIS

	use OS2::REXX;
	$ydb = load OS2::REXX "ydbautil" or die "Cannot load: $!";
	@pid = $ydb->RxProcId();
	REXX_call {
	  tie $s, OS2::REXX, "TEST";
	  $s = 1;
	};

=head1 DESCRIPTION

=head2 Load REXX DLL

	$dll = load OS2::REXX NAME [, WHERE];

NAME is DLL name, without path and extension.

Directories are searched WHERE first (list of dirs), then environment
paths PERL5REXX, PERLREXX, PATH or, as last resort, OS/2-ish search 
is performed in default DLL path (without adding paths and extensions).

The DLL is not unloaded when the variable dies.

Returns DLL object reference, or undef on failure.

=head2 Define function prefix:

	$dll->prefix(NAME);

Define the prefix of external functions, prepended to the function
names used within your program, when looking for the entries in the
DLL.

=head2 Example

		$dll = load OS2::REXX "RexxBase";
		$dll->prefix("RexxBase_");
		$dll->Init();

is the same as

		$dll = load OS2::REXX "RexxBase";
		$dll->RexxBase_Init();

=head2 Define queue:

	$dll->queue(NAME);

Define the name of the REXX queue passed to all external
functions of this module. Defaults to "SESSION".

Check for functions (optional):

	BOOL = $dll->find(NAME [, NAME [, ...]]);

Returns true if all functions are available.

=head2 Call external REXX function:

	$dll->function(arguments);

Returns the return string if the return code is 0, else undef.
Dies with error message if the function is not available.

=head1 Accessing REXX-runtime

While calling functions with REXX signature does not require the presence
of the system REXX DLL, there are some actions which require REXX-runtime 
present. Among them is the access to REXX variables by name.

One enables REXX runtime by bracketing your code by

	REXX_call BLOCK;

(trailing semicolon required!) or

	REXX_call \&subroutine_name;

Inside such a call one has access to REXX variables (see below).

An alternative way to execute code inside a REXX compartment is

	REXX_eval EXPR;
	REXX_eval_with EXPR, 
		subroutine_name_in_REXX => \&Perl_subroutine

Here C<EXPR> is a REXX code to run; to execute Perl code one needs to put
it inside Perl_subroutine(), and call this subroutine from REXX, as in

	REXX_eval_with <<EOE, foo => sub { 123 * shift };
	  say foo(2)
	EOE

If one needs more Perl subroutines available, one can "import" them into
REXX from inside Perl_subroutine(); since REXX is not case-sensitive,
the names should be uppercased.

	use OS2::REXX 'register';

	sub BAR { 123 + shift}
	sub BAZ { 789 }
	sub importer { register qw(BAR BAZ) }

	REXX_eval_with <<'EOE', importer => \&importer;
	  call importer
	  say bar(34)
	  say baz()
	EOE

=head2 Bind scalar variable to REXX variable:

	tie $var, OS2::REXX, "NAME";

=head2 Bind array variable to REXX stem variable:

	tie @var, OS2::REXX, "NAME.";

Only scalar operations work so far. No array assignments, no array
operations, ... FORGET IT.

=head2 Bind hash array variable to REXX stem variable:

	tie %var, OS2::REXX, "NAME.";

To access all visible REXX variables via hash array, bind to "";

No array assignments. No array operations, other than hash array
operations. Just like the *dbm based implementations.

For the usual REXX stem variables, append a "." to the name,
as shown above. If the hash key is part of the stem name, for
example if you bind to "", you cannot use lower case in the stem
part of the key and it is subject to character set restrictions.

=head2 Erase individual REXX variables (bound or not):

	OS2::REXX::drop("NAME" [, "NAME" [, ...]]);

=head2 Erase REXX variables with given stem (bound or not):

	OS2::REXX::dropall("STEM" [, "STEM" [, ...]]);

=head2 Make Perl functions available in REXX:

	OS2::REXX::register("NAME" [, "NAME" [, ...]]);

Since REXX is not case-sensitive, the names should be uppercase.

=head1 Subcommand handlers

By default, the executed REXX code runs without any default subcommand
handler present.  A subcommand handler named C<PERLEVAL> is defined, but
not made a default.  Use C<ADDRESS PERLEVAL> REXX command to make it a default
handler; alternatively, use C<ADDRESS Handler WhatToDo> to direct a command
to the handler you like.

Experiments show that the handler C<CMD> is also available; probably it is
provided by the REXX runtime.

=head1 Interfacing from REXX to Perl

This module provides an interface from Perl to REXX, and from REXX-inside-Perl
back to Perl.  There is an alternative scenario which allows usage of Perl
from inside REXX.

A DLL F<PerlRexx> provides an API to Perl as REXX functions

  PERL
  PERLTERM
  PERLINIT
  PERLEXIT
  PERLEVAL
  PERLLASTERROR
  PERLEXPORTALL
  PERLDROPALL
  PERLDROPALLEXIT

A subcommand handler C<PERLEVALSUBCOMMAND> can also be registered.  Calling
the function PERLEXPORTALL() exports all these functions, as well as
exports this subcommand handler under the name C<EVALPERL>.  PERLDROPALL()
inverts this action (and unloads PERLEXPORTALL() as well).  In particular

  rc = RxFuncAdd("PerlExportAll", 'PerlRexx', "PERLEXPORTALL")
  rc = PerlExportAll()
  res = PERLEVAL(perlarg)
  ADDRESS EVALPERL perlarg1
  rc = PerlDropAllExit()

loads all the functions above, evals the Perl code in the REXX variable
C<perlarg>, putting the result into the REXX variable C<res>,
then evals the Perl code in the REXX variable C<perlarg1>, and, finally,
drops the loaded functions and the subcommand handler, deinitializes
the Perl interpreter, and exits the Perl's C runtime library.

PERLEXIT() or PERLDROPALLEXIT() should be called as the last command of
the REXX program.  (This is considered as a bug.)  Their purpose is to flush
all the output buffers of the Perl's C runtime library.

C<PERLLASTERROR> gives the reason for the failure of the last PERLEVAL().
It is useful inside C<signal on syntax> handler.  PERLINIT() and PERLTERM()
initialize and deinitialize the Perl interpreter.

C<PERLEVAL(string)> initializes the Perl interpreter (if needed), and
evaluates C<string> as Perl code.  The result is returned to REXX stringified,
undefined result is considered as failure.

C<PERL(string)> does the same as C<PERLEVAL(string)> wrapped by calls to
PERLINIT() and PERLEXIT().

=head1 NOTES

Note that while function and variable names are case insensitive in the
REXX language, function names exported by a DLL and the REXX variables
(as seen by Perl through the chosen API) are all case sensitive!

Most REXX DLLs export function names all upper case, but there are a
few which export mixed case names (such as RxExtras). When trying to
find the entry point, both exact case and all upper case are searched.
If the DLL exports "RxNap", you have to specify the exact case, if it
exports "RXOPEN", you can use any case.

To avoid interfering with subroutine names defined by Perl (DESTROY)
or used within the REXX module (prefix, find), it is best to use mixed
case and to avoid lowercase only or uppercase only names when calling
REXX functions. Be consistent. The same function written in different
ways results in different Perl stubs.

There is no REXX interpolation on variable names, so the REXX variable
name TEST.ONE is not affected by some other REXX variable ONE. And it
is not the same variable as TEST.one!

You cannot call REXX functions which are not exported by the DLL.
While most DLLs export all their functions, some, like RxFTP, export
only "...LoadFuncs", which registers the functions within REXX only.

You cannot call 16-bit DLLs. The few interesting ones I found
(FTP,NETB,APPC) do not export their functions.

I do not know whether the REXX API is reentrant with respect to
exceptions (signals) when the REXX top-level exception handler is
overridden. So unless you know better than I do, do not access REXX
variables (probably tied to Perl variables) or call REXX functions
which access REXX queues or REXX variables in signal handlers.

See F<t/rx*.t> and the next section for examples.

=head1 EXAMPLE

 use OS2::REXX;

 sub Ender::DESTROY { $vrexx->VExit; print "Exiting...\n" }

 $vrexx = OS2::REXX->load('VREXX');
 REXX_call {			# VOpenWindow takes a stem
   local $SIG{TERM} = sub {die}; # enable Ender::DESTROY
   local $SIG{INT} = sub {die};	# enable Ender::DESTROY

   $code = $vrexx->VInit;
   print "Init code = `$code'\n";
   die "error initializing VREXX" if $code eq 'ERROR';

   my $ender = bless [], 'Ender'; # Call Ender::DESTROY on exit

   print "VREXX Version ", $vrexx->VGetVersion, "\n";

   tie %pos, 'OS2::REXX', 'POS.' or die;
   %pos = ( LEFT   => 0, RIGHT  => 7, TOP    => 5, BOTTOM => 0 );

   $id = $vrexx->VOpenWindow('To disconnect:', 'WHITE', 'POS');
   $vrexx->VForeColor($id, 'BLACK');
   $vrexx->VSetFont($id, 'TIME', '30');
   $tlim = time + 60;
   while ( ($r = $tlim - time) >= 0 ) {
     $vrexx->VClearWindow($id);
     $vrexx->VSay($id, 100, 50, (sprintf "%02i:%02i", int($r/60),
                                                              $r % 60));
     sleep 1;
   }
   print "Close code = `$res'\n" if $res = $vrexx->VCloseWindow($id);
 };



=head1 ENVIRONMENT

If C<PERL_REXX_DEBUG> is set, prints trace info on calls to REXX runtime
environment.

=head1 AUTHOR

Andreas Kaiser ak@ananke.s.bawue.de, with additions by Ilya Zakharevich
ilya@math.ohio-state.edu.

=head1 SEE ALSO

L<OS2::DLL>.

=cut
