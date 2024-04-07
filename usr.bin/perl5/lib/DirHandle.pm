package DirHandle;

our $VERSION = '1.05';

=head1 NAME 

DirHandle - (obsolete) supply object methods for directory handles

=head1 SYNOPSIS

    # recommended approach since Perl 5.6: do not use DirHandle
    if (opendir my $d, '.') {
        while (readdir $d) { something($_); }
        rewind $d;
        while (readdir $d) { something_else($_); }
    }

    # how you would use this module if you were going to
    use DirHandle;
    if (my $d = DirHandle->new(".")) {
        while (defined($_ = $d->read)) { something($_); }
        $d->rewind;
        while (defined($_ = $d->read)) { something_else($_); }
    }

=head1 DESCRIPTION

B<There is no reason to use this module nowadays.>

The C<DirHandle> method provide an alternative interface to the
opendir(), closedir(), readdir(), and rewinddir() functions.

Up to Perl 5.5, opendir() could not autovivify a directory handle from
C<undef>, so using a lexical handle required using a function from L<Symbol>
to create an anonymous glob, which took a separate step.
C<DirHandle> encapsulates this, which allowed cleaner code than opendir().
Since Perl 5.6, opendir() alone has been all you need for lexical handles.

=cut

require 5.000;
use Carp;
use Symbol;

sub new {
    @_ >= 1 && @_ <= 2 or croak 'usage: DirHandle->new( [DIRNAME] )';
    my $class = shift;
    my $dh = gensym;
    if (@_) {
	DirHandle::open($dh, $_[0])
	    or return undef;
    }
    bless $dh, $class;
}

sub DESTROY {
    my ($dh) = @_;
    # Don't warn about already being closed as it may have been closed 
    # correctly, or maybe never opened at all.
    local($., $@, $!, $^E, $?);
    no warnings 'io';
    closedir($dh);
}

sub open {
    @_ == 2 or croak 'usage: $dh->open(DIRNAME)';
    my ($dh, $dirname) = @_;
    opendir($dh, $dirname);
}

sub close {
    @_ == 1 or croak 'usage: $dh->close()';
    my ($dh) = @_;
    closedir($dh);
}

sub read {
    @_ == 1 or croak 'usage: $dh->read()';
    my ($dh) = @_;
    readdir($dh);
}

sub rewind {
    @_ == 1 or croak 'usage: $dh->rewind()';
    my ($dh) = @_;
    rewinddir($dh);
}

1;
