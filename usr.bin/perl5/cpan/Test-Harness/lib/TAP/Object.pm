package TAP::Object;

use strict;
use warnings;

=head1 NAME

TAP::Object - Base class that provides common functionality to all C<TAP::*> modules

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

    package TAP::Whatever;

    use strict;

    use base 'TAP::Object';

    # new() implementation by TAP::Object
    sub _initialize {
        my ( $self, @args) = @_;
        # initialize your object
        return $self;
    }

    # ... later ...
    my $obj = TAP::Whatever->new(@args);

=head1 DESCRIPTION

C<TAP::Object> provides a default constructor and exception model for all
C<TAP::*> classes.  Exceptions are raised using L<Carp>.

=head1 METHODS

=head2 Class Methods

=head3 C<new>

Create a new object.  Any arguments passed to C<new> will be passed on to the
L</_initialize> method.  Returns a new object.

=cut

sub new {
    my $class = shift;
    my $self = bless {}, $class;
    return $self->_initialize(@_);
}

=head2 Instance Methods

=head3 C<_initialize>

Initializes a new object.  This method is a stub by default, you should override
it as appropriate.

I<Note:> L</new> expects you to return C<$self> or raise an exception.  See
L</_croak>, and L<Carp>.

=cut

sub _initialize {
    return $_[0];
}

=head3 C<_croak>

Raise an exception using C<croak> from L<Carp>, eg:

    $self->_croak( 'why me?', 'aaarrgh!' );

May also be called as a I<class> method.

    $class->_croak( 'this works too' );

=cut

sub _croak {
    my $proto = shift;
    require Carp;
    Carp::croak(@_);
    return;
}

=head3 C<_confess>

Raise an exception using C<confess> from L<Carp>, eg:

    $self->_confess( 'why me?', 'aaarrgh!' );

May also be called as a I<class> method.

    $class->_confess( 'this works too' );

=cut

sub _confess {
    my $proto = shift;
    require Carp;
    Carp::confess(@_);
    return;
}

=head3 C<_construct>

Create a new instance of the specified class.

=cut

sub _construct {
    my ( $self, $class, @args ) = @_;

    $self->_croak("Bad module name $class")
      unless $class =~ /^ \w+ (?: :: \w+ ) *$/x;

    unless ( $class->can('new') ) {
        local $@;
        eval "require $class";
        $self->_croak("Can't load $class: $@") if $@;
    }

    return $class->new(@args);
}

=head3 C<mk_methods>

Create simple getter/setters.

 __PACKAGE__->mk_methods(@method_names);

=cut

sub mk_methods {
    my ( $class, @methods ) = @_;
    for my $method_name (@methods) {
        my $method = "${class}::$method_name";
        no strict 'refs';
        *$method = sub {
            my $self = shift;
            $self->{$method_name} = shift if @_;
            return $self->{$method_name};
        };
    }
}

1;

