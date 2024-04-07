package VMS::DCLsym;

use Carp;
use DynaLoader;
use strict;

# Package globals
our @ISA = ( 'DynaLoader' );
our $VERSION = '1.09';              # remember to update version in POD!
my(%Locsyms) = ( ':ID' => 'LOCAL' );
my(%Gblsyms) = ( ':ID' => 'GLOBAL');
my $DoCache = 1;
my $Cache_set = 0;


#====> OO methods

sub new {
  my($pkg,$type) = @_;
  $type ||= 'LOCAL';
  $type = 'LOCAL' unless $type eq 'GLOBAL';
  bless { TYPE => $type }, $pkg;
}

sub DESTROY { }

sub getsym {
  my($self,$name) = @_;
  my($val,$table);

  if (($val,$table) = _getsym($name)) {
    if ($table eq 'GLOBAL') { $Gblsyms{$name} = $val; }
    else                    { $Locsyms{$name} = $val; }
  }
  wantarray ? ($val,$table) : $val;
}

sub setsym {
  my($self,$name,$val,$table) = @_;

  $table = $self->{TYPE} unless $table;
  if (_setsym($name,$val,$table)) {
    if ($table eq 'GLOBAL') { $Gblsyms{$name} = $val; }
    else                    { $Locsyms{$name} = $val; }
    1;
  }
  else { 0; }
}
  
sub delsym {
  my($self,$name,$table) = @_;

  $table = $self->{TYPE} unless $table;
  if (_delsym($name,$table)) {
    if ($table eq 'GLOBAL') { delete $Gblsyms{$name}; }
    else                    { delete $Locsyms{$name}; }
    1;
  }
  else { 0; }
}

sub clearcache {
  my($self,$perm) = @_;
  my($old);

  $Cache_set = 0;
  %Locsyms = ( ':ID' => 'LOCAL');
  %Gblsyms = ( ':ID' => 'GLOBAL');
  $old = $DoCache;
  $DoCache = $perm if defined($perm);
  $old;
}

#====> TIEHASH methods

sub TIEHASH {
  shift->new(@_);
}

sub FETCH {
  my($self,$name) = @_;
  if    ($name eq ':GLOBAL') { $self->{TYPE} eq 'GLOBAL'; }
  elsif ($name eq ':LOCAL' ) { $self->{TYPE} eq 'LOCAL';  }
  else                       { scalar($self->getsym($name)); }
}

sub STORE {
  my($self,$name,$val) = @_;
  if    ($name eq ':GLOBAL') { $self->{TYPE} = 'GLOBAL'; }
  elsif ($name eq ':LOCAL' ) { $self->{TYPE} = 'LOCAL';  }
  else                       { $self->setsym($name,$val); }
}

sub DELETE {
  my($self,$name) = @_;

  $self->delsym($name);
}

sub FIRSTKEY {
  my($self) = @_;
  my($name,$eqs,$val);

  if (!$DoCache || !$Cache_set) {
    # We should eventually replace this with a C routine which walks the
    # CLI symbol table directly.  If I ever get 'hold of an I&DS manual . . .
    open(P, '-|', 'Show Symbol *');
    while (<P>) {
      ($name,$eqs,$val) = /^\s+(\S+) (=+) (.+)/
        or carp "VMS::DCLsym: unparseable line $_";
      $name =~ s#\*##;
      $val =~ s/"(.*)"$/$1/ or $val =~ s/^(\S+).*/$1/;
      if ($eqs eq '==') { $Gblsyms{$name} = $val; }
      else              { $Locsyms{$name} = $val; }
    }
    close P;
    $Cache_set = 1;
  }
  $self ->{IDX} = 0;
  $self->{CACHE} = $self->{TYPE} eq 'GLOBAL' ? \%Gblsyms : \%Locsyms;
  while (($name,$val) = each(%{$self->{CACHE}}) and !defined($name)) {
    if ($self->{CACHE}{':ID'} eq 'GLOBAL') { return undef; }
    $self->{CACHE} = \%Gblsyms;
  }
  $name;
}

sub NEXTKEY {
  my($self) = @_;
  my($name,$val);

  while (($name,$val) = each(%{$self->{CACHE}}) and !defined($name)) {
    if ($self->{CACHE}{':ID'} eq 'GLOBAL') { return undef; }
    $self->{CACHE} = \%Gblsyms;
  }
  $name;
}


sub EXISTS { defined($_[0]->FETCH(@_)) ? 1 : 0 }

sub CLEAR { }


bootstrap VMS::DCLsym;

1;

__END__

=head1 NAME

VMS::DCLsym - Perl extension to manipulate DCL symbols

=head1 SYNOPSIS

  tie %allsyms, VMS::DCLsym;
  tie %cgisyms, VMS::DCLsym, 'GLOBAL';


  $handle = new VMS::DCLsym;
  $value = $handle->getsym($name);
  $handle->setsym($name, $value, 'GLOBAL')
      or die "Can't create symbol: $!\n";
  $handle->delsym($name, 'LOCAL') or die "Can't delete symbol: $!\n";
  $handle->clearcache();

=head1 DESCRIPTION

The VMS::DCLsym extension provides access to DCL symbols using a
tied hash interface.  This allows Perl scripts to manipulate symbols in
a manner similar to the way in which logical names are manipulated via
the built-in C<%ENV> hash.  Alternatively, one can call methods in this
package directly to read, create, and delete symbols.

=head2 Tied hash interface

This interface lets you treat the DCL symbol table as a Perl associative array,
in which the key of each element is the symbol name, and the value of the
element is that symbol's value.  Case is not significant in the key string, as
DCL converts symbol names to uppercase, but it is significant in the value
string.  All of the usual operations on associative arrays are supported. 
Reading an element retrieves the current value of the symbol, assigning to it
defines a new symbol (or overwrites the old value of an existing symbol), and
deleting an element deletes the corresponding symbol.  Setting an element to
C<undef>, or C<undef>ing it directly, sets the corresponding symbol to the null
string. You may also read the special keys ':GLOBAL' and ':LOCAL' to find out
whether a default symbol table has been specified for this hash (see the next
paragraph), or set either or these keys to specify a default symbol table.

When you call the C<tie> function to bind an associative array to this package,
you may specify as an optional argument the symbol table in which you wish to
create and delete symbols.  If the argument is the string 'GLOBAL', then the
global symbol table is used; any other string causes the local symbol table to
be used.  Note that this argument does not affect attempts to read symbols; if
a symbol with the specified name exists in the local symbol table, it is always
returned in preference to a symbol by the same name in the global symbol table.

=head2 Object interface

Although it's less convenient in some ways than the tied hash interface, you
can also call methods directly to manipulate individual symbols.  In some
cases, this allows you finer control than using a tied hash aggregate.  The
following methods are supported:

=over 4

=item new

This creates a C<VMS::DCLsym> object which can be used as a handle for later
method calls.  The single optional argument specifies the symbol table used
by default in future method calls, in the same way as the optional argument to
C<tie> described above.

=item getsym

If called in a scalar context, C<getsym> returns the value of the symbol whose
name is given as the argument to the call, or C<undef> if no such symbol
exists.  Symbols in the local symbol table are always used in preference to
symbols in the global symbol table.  If called in a list context, C<getsym>
returns a two-element list, whose first element is the value of the symbol, and
whose second element is the string 'GLOBAL' or 'LOCAL', indicating the table
from which the symbol's value was read.

=item setsym

The first two arguments taken by this method are the name of the symbol and the
value which should be assigned to it.  The optional third argument is a string
specifying the symbol table to be used; 'GLOBAL' specifies the global symbol
table, and any other string specifies the local symbol table.  If this argument
is omitted, the default symbol table for the object is used.  C<setsym> returns
TRUE if successful, and FALSE otherwise.

=item delsym

This method deletes the symbol whose name is given as the first argument.  The
optional second argument specifies the symbol table, as described above under
C<setsym>.  It returns TRUE if the symbol was successfully deleted, and FALSE
if it was not.

=item clearcache

Because of the overhead associated with obtaining the list of defined symbols
for the tied hash iterator, it is only done once, and the list is reused for
subsequent iterations.  Changes to symbols made through this package are
recorded, but in the rare event that someone changes the process' symbol table
from outside (as is possible using some software from the net), the iterator
will be out of sync with the symbol table.  If you expect this to happen, you
can reset the cache by calling this method.  In addition, if you pass a FALSE
value as the first argument, caching will be disabled.  It can be re-enabled
later by calling C<clearcache> again with a TRUE value as the first argument.
It returns TRUE or FALSE to indicate whether caching was previously enabled or
disabled, respectively.

This method is a stopgap until we can incorporate code into this extension to
traverse the process' symbol table directly, so it may disappear in a future
version of this package.

=back

=head1 AUTHOR

Charles Bailey  bailey@newman.upenn.edu

=head1 VERSION

1.09

=head1 BUGS

The list of symbols for the iterator is assembled by spawning off a
subprocess, which can be slow.  Ideally, we should just traverse the
process' symbol table directly from C.

