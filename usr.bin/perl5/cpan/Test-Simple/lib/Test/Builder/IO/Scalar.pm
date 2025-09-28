package Test::Builder::IO::Scalar;


=head1 NAME

Test::Builder::IO::Scalar - A copy of IO::Scalar for Test::Builder

=head1 DESCRIPTION

This is a copy of L<IO::Scalar> which ships with L<Test::Builder> to
support scalar references as filehandles on Perl 5.6.  Newer
versions of Perl simply use C<open()>'s built in support.

L<Test::Builder> can not have dependencies on other modules without
careful consideration, so its simply been copied into the distribution.

=head1 COPYRIGHT and LICENSE

This file came from the "IO-stringy" Perl5 toolkit.

Copyright (c) 1996 by Eryq.  All rights reserved.
Copyright (c) 1999,2001 by ZeeGee Software Inc.  All rights reserved.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.


=cut

# This is copied code, I don't care.
##no critic

use Carp;
use strict;
use vars qw($VERSION @ISA);
use IO::Handle;

use 5.005;

### The package version, both in 1.23 style *and* usable by MakeMaker:
$VERSION = "2.114";

### Inheritance:
@ISA = qw(IO::Handle);

#==============================

=head2 Construction

=over 4

=cut

#------------------------------

=item new [ARGS...]

I<Class method.>
Return a new, unattached scalar handle.
If any arguments are given, they're sent to open().

=cut

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $self = bless \do { local *FH }, $class;
    tie *$self, $class, $self;
    $self->open(@_);   ### open on anonymous by default
    $self;
}
sub DESTROY {
    shift->close;
}

#------------------------------

=item open [SCALARREF]

I<Instance method.>
Open the scalar handle on a new scalar, pointed to by SCALARREF.
If no SCALARREF is given, a "private" scalar is created to hold
the file data.

Returns the self object on success, undefined on error.

=cut

sub open {
    my ($self, $sref) = @_;

    ### Sanity:
    defined($sref) or do {my $s = ''; $sref = \$s};
    (ref($sref) eq "SCALAR") or croak "open() needs a ref to a scalar";

    ### Setup:
    *$self->{Pos} = 0;          ### seek position
    *$self->{SR}  = $sref;      ### scalar reference
    $self;
}

#------------------------------

=item opened

I<Instance method.>
Is the scalar handle opened on something?

=cut

sub opened {
    *{shift()}->{SR};
}

#------------------------------

=item close

I<Instance method.>
Disassociate the scalar handle from its underlying scalar.
Done automatically on destroy.

=cut

sub close {
    my $self = shift;
    %{*$self} = ();
    1;
}

=back

=cut



#==============================

=head2 Input and output

=over 4

=cut


#------------------------------

=item flush

I<Instance method.>
No-op, provided for OO compatibility.

=cut

sub flush { "0 but true" }

#------------------------------

=item getc

I<Instance method.>
Return the next character, or undef if none remain.

=cut

sub getc {
    my $self = shift;

    ### Return undef right away if at EOF; else, move pos forward:
    return undef if $self->eof;
    substr(${*$self->{SR}}, *$self->{Pos}++, 1);
}

#------------------------------

=item getline

I<Instance method.>
Return the next line, or undef on end of string.
Can safely be called in an array context.
Currently, lines are delimited by "\n".

=cut

sub getline {
    my $self = shift;

    ### Return undef right away if at EOF:
    return undef if $self->eof;

    ### Get next line:
    my $sr = *$self->{SR};
    my $i  = *$self->{Pos};	        ### Start matching at this point.

    ### Minimal impact implementation!
    ### We do the fast fast thing (no regexps) if using the
    ### classic input record separator.

    ### Case 1: $/ is undef: slurp all...
    if    (!defined($/)) {
	*$self->{Pos} = length $$sr;
        return substr($$sr, $i);
    }

    ### Case 2: $/ is "\n": zoom zoom zoom...
    elsif ($/ eq "\012") {

        ### Seek ahead for "\n"... yes, this really is faster than regexps.
        my $len = length($$sr);
        for (; $i < $len; ++$i) {
           last if ord (substr ($$sr, $i, 1)) == 10;
        }

        ### Extract the line:
        my $line;
        if ($i < $len) {                ### We found a "\n":
            $line = substr ($$sr, *$self->{Pos}, $i - *$self->{Pos} + 1);
            *$self->{Pos} = $i+1;            ### Remember where we finished up.
        }
        else {                          ### No "\n"; slurp the remainder:
            $line = substr ($$sr, *$self->{Pos}, $i - *$self->{Pos});
            *$self->{Pos} = $len;
        }
        return $line;
    }

    ### Case 3: $/ is ref to int. Do fixed-size records.
    ###        (Thanks to Dominique Quatravaux.)
    elsif (ref($/)) {
        my $len = length($$sr);
		my $i = ${$/} + 0;
		my $line = substr ($$sr, *$self->{Pos}, $i);
		*$self->{Pos} += $i;
        *$self->{Pos} = $len if (*$self->{Pos} > $len);
		return $line;
    }

    ### Case 4: $/ is either "" (paragraphs) or something weird...
    ###         This is Graham's general-purpose stuff, which might be
    ###         a tad slower than Case 2 for typical data, because
    ###         of the regexps.
    else {
        pos($$sr) = $i;

	### If in paragraph mode, skip leading lines (and update i!):
        length($/) or
	    (($$sr =~ m/\G\n*/g) and ($i = pos($$sr)));

        ### If we see the separator in the buffer ahead...
        if (length($/)
	    ?  $$sr =~ m,\Q$/\E,g          ###   (ordinary sep) TBD: precomp!
            :  $$sr =~ m,\n\n,g            ###   (a paragraph)
            ) {
            *$self->{Pos} = pos $$sr;
            return substr($$sr, $i, *$self->{Pos}-$i);
        }
        ### Else if no separator remains, just slurp the rest:
        else {
            *$self->{Pos} = length $$sr;
            return substr($$sr, $i);
        }
    }
}

#------------------------------

=item getlines

I<Instance method.>
Get all remaining lines.
It will croak() if accidentally called in a scalar context.

=cut

sub getlines {
    my $self = shift;
    wantarray or croak("can't call getlines in scalar context!");
    my ($line, @lines);
    push @lines, $line while (defined($line = $self->getline));
    @lines;
}

#------------------------------

=item print ARGS...

I<Instance method.>
Print ARGS to the underlying scalar.

B<Warning:> this continues to always cause a seek to the end
of the string, but if you perform seek()s and tell()s, it is
still safer to explicitly seek-to-end before subsequent print()s.

=cut

sub print {
    my $self = shift;
    *$self->{Pos} = length(${*$self->{SR}} .= join('', @_) . (defined($\) ? $\ : ""));
    1;
}
sub _unsafe_print {
    my $self = shift;
    my $append = join('', @_) . $\;
    ${*$self->{SR}} .= $append;
    *$self->{Pos}   += length($append);
    1;
}
sub _old_print {
    my $self = shift;
    ${*$self->{SR}} .= join('', @_) . $\;
    *$self->{Pos} = length(${*$self->{SR}});
    1;
}


#------------------------------

=item read BUF, NBYTES, [OFFSET]

I<Instance method.>
Read some bytes from the scalar.
Returns the number of bytes actually read, 0 on end-of-file, undef on error.

=cut

sub read {
    my $self = $_[0];
    my $n    = $_[2];
    my $off  = $_[3] || 0;

    my $read = substr(${*$self->{SR}}, *$self->{Pos}, $n);
    $n = length($read);
    *$self->{Pos} += $n;
    ($off ? substr($_[1], $off) : $_[1]) = $read;
    return $n;
}

#------------------------------

=item write BUF, NBYTES, [OFFSET]

I<Instance method.>
Write some bytes to the scalar.

=cut

sub write {
    my $self = $_[0];
    my $n    = $_[2];
    my $off  = $_[3] || 0;

    my $data = substr($_[1], $off, $n);
    $n = length($data);
    $self->print($data);
    return $n;
}

#------------------------------

=item sysread BUF, LEN, [OFFSET]

I<Instance method.>
Read some bytes from the scalar.
Returns the number of bytes actually read, 0 on end-of-file, undef on error.

=cut

sub sysread {
  my $self = shift;
  $self->read(@_);
}

#------------------------------

=item syswrite BUF, NBYTES, [OFFSET]

I<Instance method.>
Write some bytes to the scalar.

=cut

sub syswrite {
  my $self = shift;
  $self->write(@_);
}

=back

=cut


#==============================

=head2 Seeking/telling and other attributes

=over 4

=cut


#------------------------------

=item autoflush

I<Instance method.>
No-op, provided for OO compatibility.

=cut

sub autoflush {}

#------------------------------

=item binmode

I<Instance method.>
No-op, provided for OO compatibility.

=cut

sub binmode {}

#------------------------------

=item clearerr

I<Instance method.>  Clear the error and EOF flags.  A no-op.

=cut

sub clearerr { 1 }

#------------------------------

=item eof

I<Instance method.>  Are we at end of file?

=cut

sub eof {
    my $self = shift;
    (*$self->{Pos} >= length(${*$self->{SR}}));
}

#------------------------------

=item seek OFFSET, WHENCE

I<Instance method.>  Seek to a given position in the stream.

=cut

sub seek {
    my ($self, $pos, $whence) = @_;
    my $eofpos = length(${*$self->{SR}});

    ### Seek:
    if    ($whence == 0) { *$self->{Pos} = $pos }             ### SEEK_SET
    elsif ($whence == 1) { *$self->{Pos} += $pos }            ### SEEK_CUR
    elsif ($whence == 2) { *$self->{Pos} = $eofpos + $pos}    ### SEEK_END
    else                 { croak "bad seek whence ($whence)" }

    ### Fixup:
    if (*$self->{Pos} < 0)       { *$self->{Pos} = 0 }
    if (*$self->{Pos} > $eofpos) { *$self->{Pos} = $eofpos }
    return 1;
}

#------------------------------

=item sysseek OFFSET, WHENCE

I<Instance method.> Identical to C<seek OFFSET, WHENCE>, I<q.v.>

=cut

sub sysseek {
    my $self = shift;
    $self->seek (@_);
}

#------------------------------

=item tell

I<Instance method.>
Return the current position in the stream, as a numeric offset.

=cut

sub tell { *{shift()}->{Pos} }

#------------------------------

=item  use_RS [YESNO]

I<Instance method.>
B<Deprecated and ignored.>
Obey the current setting of $/, like IO::Handle does?
Default is false in 1.x, but cold-welded true in 2.x and later.

=cut

sub use_RS {
    my ($self, $yesno) = @_;
    carp "use_RS is deprecated and ignored; \$/ is always consulted\n";
 }

#------------------------------

=item setpos POS

I<Instance method.>
Set the current position, using the opaque value returned by C<getpos()>.

=cut

sub setpos { shift->seek($_[0],0) }

#------------------------------

=item getpos

I<Instance method.>
Return the current position in the string, as an opaque object.

=cut

*getpos = \&tell;


#------------------------------

=item sref

I<Instance method.>
Return a reference to the underlying scalar.

=cut

sub sref { *{shift()}->{SR} }


#------------------------------
# Tied handle methods...
#------------------------------

# Conventional tiehandle interface:
sub TIEHANDLE {
    ((defined($_[1]) && UNIVERSAL::isa($_[1], __PACKAGE__))
     ? $_[1]
     : shift->new(@_));
}
sub GETC      { shift->getc(@_) }
sub PRINT     { shift->print(@_) }
sub PRINTF    { shift->print(sprintf(shift, @_)) }
sub READ      { shift->read(@_) }
sub READLINE  { wantarray ? shift->getlines(@_) : shift->getline(@_) }
sub WRITE     { shift->write(@_); }
sub CLOSE     { shift->close(@_); }
sub SEEK      { shift->seek(@_); }
sub TELL      { shift->tell(@_); }
sub EOF       { shift->eof(@_); }
sub FILENO    { -1 }

#------------------------------------------------------------

1;

__END__



=back

=cut


=head1 WARNINGS

Perl's TIEHANDLE spec was incomplete prior to 5.005_57;
it was missing support for C<seek()>, C<tell()>, and C<eof()>.
Attempting to use these functions with an IO::Scalar will not work
prior to 5.005_57. IO::Scalar will not have the relevant methods
invoked; and even worse, this kind of bug can lie dormant for a while.
If you turn warnings on (via C<$^W> or C<perl -w>),
and you see something like this...

    attempt to seek on unopened filehandle

...then you are probably trying to use one of these functions
on an IO::Scalar with an old Perl.  The remedy is to simply
use the OO version; e.g.:

    $SH->seek(0,0);    ### GOOD: will work on any 5.005
    seek($SH,0,0);     ### WARNING: will only work on 5.005_57 and beyond


=head1 VERSION

$Id: Scalar.pm,v 1.6 2005/02/10 21:21:53 dfs Exp $


=head1 AUTHORS

=head2 Primary Maintainer

David F. Skoll (F<dfs@roaringpenguin.com>).

=head2 Principal author

Eryq (F<eryq@zeegee.com>).
President, ZeeGee Software Inc (F<http://www.zeegee.com>).


=head2 Other contributors

The full set of contributors always includes the folks mentioned
in L<IO::Stringy/"CHANGE LOG">.  But just the same, special
thanks to the following individuals for their invaluable contributions
(if I've forgotten or misspelled your name, please email me!):

I<Andy Glew,>
for contributing C<getc()>.

I<Brandon Browning,>
for suggesting C<opened()>.

I<David Richter,>
for finding and fixing the bug in C<PRINTF()>.

I<Eric L. Brine,>
for his offset-using read() and write() implementations.

I<Richard Jones,>
for his patches to massively improve the performance of C<getline()>
and add C<sysread> and C<syswrite>.

I<B. K. Oxley (binkley),>
for stringification and inheritance improvements,
and sundry good ideas.

I<Doug Wilson,>
for the IO::Handle inheritance and automatic tie-ing.


=head1 SEE ALSO

L<IO::String>, which is quite similar but which was designed
more-recently and with an IO::Handle-like interface in mind,
so you could mix OO- and native-filehandle usage without using tied().

I<Note:> as of version 2.x, these classes all work like
their IO::Handle counterparts, so we have comparable
functionality to IO::String.

=cut

