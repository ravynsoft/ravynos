package Tie::StdHandle; 

use strict;

use Tie::Handle;
our @ISA = 'Tie::Handle';
our $VERSION = '4.6';

=head1 NAME

Tie::StdHandle - base class definitions for tied handles

=head1 SYNOPSIS

    package NewHandle;
    require Tie::Handle;

    @ISA = qw(Tie::Handle);

    sub READ { ... }		# Provide a needed method
    sub TIEHANDLE { ... }	# Overrides inherited method


    package main;

    tie *FH, 'NewHandle';

=head1 DESCRIPTION

The B<Tie::StdHandle> package provide most methods for file handles described
in L<perltie> (the exceptions are C<UNTIE> and C<DESTROY>).  It causes tied
file handles to behave exactly like standard file handles and allow for
selective overwriting of methods.

=cut

sub TIEHANDLE 
{
 my $class = shift;
 my $fh    = \do { local *HANDLE};
 bless $fh,$class;
 $fh->OPEN(@_) if (@_);
 return $fh;
}

sub EOF     { eof($_[0]) }
sub TELL    { tell($_[0]) }
sub FILENO  { fileno($_[0]) }
sub SEEK    { seek($_[0],$_[1],$_[2]) }
sub CLOSE   { close($_[0]) }
sub BINMODE { &CORE::binmode(shift, @_) }

sub OPEN
{
 $_[0]->CLOSE if defined($_[0]->FILENO);
 @_ == 2 ? open($_[0], $_[1]) : open($_[0], $_[1], $_[2]);
}

sub READ     { &CORE::read(shift, \shift, @_) }
sub READLINE { my $fh = $_[0]; <$fh> }
sub GETC     { getc($_[0]) }

sub WRITE
{
 my $fh = $_[0];
 local $\; # don't print any line terminator
 print $fh substr($_[1], $_[3], $_[2]);
}


1;
