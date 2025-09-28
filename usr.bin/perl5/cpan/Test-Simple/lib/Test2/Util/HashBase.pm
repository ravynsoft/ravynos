package Test2::Util::HashBase;
use strict;
use warnings;

our $VERSION = '1.302194';

#################################################################
#                                                               #
#  This is a generated file! Do not modify this file directly!  #
#  Use hashbase_inc.pl script to regenerate this file.          #
#  The script is part of the Object::HashBase distribution.     #
#  Note: You can modify the version number above this comment   #
#  if needed, that is fine.                                     #
#                                                               #
#################################################################

{
    no warnings 'once';
    $Test2::Util::HashBase::HB_VERSION = '0.009';
    *Test2::Util::HashBase::ATTR_SUBS = \%Object::HashBase::ATTR_SUBS;
    *Test2::Util::HashBase::ATTR_LIST = \%Object::HashBase::ATTR_LIST;
    *Test2::Util::HashBase::VERSION   = \%Object::HashBase::VERSION;
    *Test2::Util::HashBase::CAN_CACHE = \%Object::HashBase::CAN_CACHE;
}


require Carp;
{
    no warnings 'once';
    $Carp::Internal{+__PACKAGE__} = 1;
}

BEGIN {
    # these are not strictly equivalent, but for out use we don't care
    # about order
    *_isa = ($] >= 5.010 && require mro) ? \&mro::get_linear_isa : sub {
        no strict 'refs';
        my @packages = ($_[0]);
        my %seen;
        for my $package (@packages) {
            push @packages, grep !$seen{$_}++, @{"$package\::ISA"};
        }
        return \@packages;
    }
}

my %SPEC = (
    '^' => {reader => 1, writer => 0, dep_writer => 1, read_only => 0, strip => 1},
    '-' => {reader => 1, writer => 0, dep_writer => 0, read_only => 1, strip => 1},
    '>' => {reader => 0, writer => 1, dep_writer => 0, read_only => 0, strip => 1},
    '<' => {reader => 1, writer => 0, dep_writer => 0, read_only => 0, strip => 1},
    '+' => {reader => 0, writer => 0, dep_writer => 0, read_only => 0, strip => 1},
);

sub import {
    my $class = shift;
    my $into  = caller;

    # Make sure we list the OLDEST version used to create this class.
    my $ver = $Test2::Util::HashBase::HB_VERSION || $Test2::Util::HashBase::VERSION;
    $Test2::Util::HashBase::VERSION{$into} = $ver if !$Test2::Util::HashBase::VERSION{$into} || $Test2::Util::HashBase::VERSION{$into} > $ver;

    my $isa = _isa($into);
    my $attr_list = $Test2::Util::HashBase::ATTR_LIST{$into} ||= [];
    my $attr_subs = $Test2::Util::HashBase::ATTR_SUBS{$into} ||= {};

    my %subs = (
        ($into->can('new') ? () : (new => \&_new)),
        (map %{$Test2::Util::HashBase::ATTR_SUBS{$_} || {}}, @{$isa}[1 .. $#$isa]),
        (
            map {
                my $p = substr($_, 0, 1);
                my $x = $_;

                my $spec = $SPEC{$p} || {reader => 1, writer => 1};

                substr($x, 0, 1) = '' if $spec->{strip};
                push @$attr_list => $x;
                my ($sub, $attr) = (uc $x, $x);

                $attr_subs->{$sub} = sub() { $attr };
                my %out = ($sub => $attr_subs->{$sub});

                $out{$attr}       = sub { $_[0]->{$attr} }                                                  if $spec->{reader};
                $out{"set_$attr"} = sub { $_[0]->{$attr} = $_[1] }                                          if $spec->{writer};
                $out{"set_$attr"} = sub { Carp::croak("'$attr' is read-only") }                             if $spec->{read_only};
                $out{"set_$attr"} = sub { Carp::carp("set_$attr() is deprecated"); $_[0]->{$attr} = $_[1] } if $spec->{dep_writer};

                %out;
            } @_
        ),
    );

    no strict 'refs';
    *{"$into\::$_"} = $subs{$_} for keys %subs;
}

sub attr_list {
    my $class = shift;

    my $isa = _isa($class);

    my %seen;
    my @list = grep { !$seen{$_}++ } map {
        my @out;

        if (0.004 > ($Test2::Util::HashBase::VERSION{$_} || 0)) {
            Carp::carp("$_ uses an inlined version of Test2::Util::HashBase too old to support attr_list()");
        }
        else {
            my $list = $Test2::Util::HashBase::ATTR_LIST{$_};
            @out = $list ? @$list : ()
        }

        @out;
    } reverse @$isa;

    return @list;
}

sub _new {
    my $class = shift;

    my $self;

    if (@_ == 1) {
        my $arg = shift;
        my $type = ref($arg);

        if ($type eq 'HASH') {
            $self = bless({%$arg}, $class)
        }
        else {
            Carp::croak("Not sure what to do with '$type' in $class constructor")
                unless $type eq 'ARRAY';

            my %proto;
            my @attributes = attr_list($class);
            while (@$arg) {
                my $val = shift @$arg;
                my $key = shift @attributes or Carp::croak("Too many arguments for $class constructor");
                $proto{$key} = $val;
            }

            $self = bless(\%proto, $class);
        }
    }
    else {
        $self = bless({@_}, $class);
    }

    $Test2::Util::HashBase::CAN_CACHE{$class} = $self->can('init')
        unless exists $Test2::Util::HashBase::CAN_CACHE{$class};

    $self->init if $Test2::Util::HashBase::CAN_CACHE{$class};

    $self;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Util::HashBase - Build hash based classes.

=head1 SYNOPSIS

A class:

    package My::Class;
    use strict;
    use warnings;

    # Generate 3 accessors
    use Test2::Util::HashBase qw/foo -bar ^baz <bat >ban +boo/;

    # Chance to initialize defaults
    sub init {
        my $self = shift;    # No other args
        $self->{+FOO} ||= "foo";
        $self->{+BAR} ||= "bar";
        $self->{+BAZ} ||= "baz";
        $self->{+BAT} ||= "bat";
        $self->{+BAN} ||= "ban";
        $self->{+BOO} ||= "boo";
    }

    sub print {
        print join ", " => map { $self->{$_} } FOO, BAR, BAZ, BAT, BAN, BOO;
    }

Subclass it

    package My::Subclass;
    use strict;
    use warnings;

    # Note, you should subclass before loading HashBase.
    use base 'My::Class';
    use Test2::Util::HashBase qw/bub/;

    sub init {
        my $self = shift;

        # We get the constants from the base class for free.
        $self->{+FOO} ||= 'SubFoo';
        $self->{+BUB} ||= 'bub';

        $self->SUPER::init();
    }

use it:

    package main;
    use strict;
    use warnings;
    use My::Class;

    # These are all functionally identical
    my $one   = My::Class->new(foo => 'MyFoo', bar => 'MyBar');
    my $two   = My::Class->new({foo => 'MyFoo', bar => 'MyBar'});
    my $three = My::Class->new(['MyFoo', 'MyBar']);

    # Readers!
    my $foo = $one->foo;    # 'MyFoo'
    my $bar = $one->bar;    # 'MyBar'
    my $baz = $one->baz;    # Defaulted to: 'baz'
    my $bat = $one->bat;    # Defaulted to: 'bat'
    # '>ban' means setter only, no reader
    # '+boo' means no setter or reader, just the BOO constant

    # Setters!
    $one->set_foo('A Foo');

    #'-bar' means read-only, so the setter will throw an exception (but is defined).
    $one->set_bar('A bar');

    # '^baz' means deprecated setter, this will warn about the setter being
    # deprecated.
    $one->set_baz('A Baz');

    # '<bat' means no setter defined at all
    # '+boo' means no setter or reader, just the BOO constant

    $one->{+FOO} = 'xxx';

=head1 DESCRIPTION

This package is used to generate classes based on hashrefs. Using this class
will give you a C<new()> method, as well as generating accessors you request.
Generated accessors will be getters, C<set_ACCESSOR> setters will also be
generated for you. You also get constants for each accessor (all caps) which
return the key into the hash for that accessor. Single inheritance is also
supported.

=head1 THIS IS A BUNDLED COPY OF HASHBASE

This is a bundled copy of L<Object::HashBase>. This file was generated using
the
C</home/exodist/perl5/perlbrew/perls/main/bin/hashbase_inc.pl>
script.

=head1 METHODS

=head2 PROVIDED BY HASH BASE

=over 4

=item $it = $class->new(%PAIRS)

=item $it = $class->new(\%PAIRS)

=item $it = $class->new(\@ORDERED_VALUES)

Create a new instance.

HashBase will not export C<new()> if there is already a C<new()> method in your
packages inheritance chain.

B<If you do not want this method you can define your own> you just have to
declare it before loading L<Test2::Util::HashBase>.

    package My::Package;

    # predeclare new() so that HashBase does not give us one.
    sub new;

    use Test2::Util::HashBase qw/foo bar baz/;

    # Now we define our own new method.
    sub new { ... }

This makes it so that HashBase sees that you have your own C<new()> method.
Alternatively you can define the method before loading HashBase instead of just
declaring it, but that scatters your use statements.

The most common way to create an object is to pass in key/value pairs where
each key is an attribute and each value is what you want assigned to that
attribute. No checking is done to verify the attributes or values are valid,
you may do that in C<init()> if desired.

If you would like, you can pass in a hashref instead of pairs. When you do so
the hashref will be copied, and the copy will be returned blessed as an object.
There is no way to ask HashBase to bless a specific hashref.

In some cases an object may only have 1 or 2 attributes, in which case a
hashref may be too verbose for your liking. In these cases you can pass in an
arrayref with only values. The values will be assigned to attributes in the
order the attributes were listed. When there is inheritance involved the
attributes from parent classes will come before subclasses.

=back

=head2 HOOKS

=over 4

=item $self->init()

This gives you the chance to set some default values to your fields. The only
argument is C<$self> with its indexes already set from the constructor.

B<Note:> Test2::Util::HashBase checks for an init using C<< $class->can('init') >>
during construction. It DOES NOT call C<can()> on the created object. Also note
that the result of the check is cached, it is only ever checked once, the first
time an instance of your class is created. This means that adding an C<init()>
method AFTER the first construction will result in it being ignored.

=back

=head1 ACCESSORS

=head2 READ/WRITE

To generate accessors you list them when using the module:

    use Test2::Util::HashBase qw/foo/;

This will generate the following subs in your namespace:

=over 4

=item foo()

Getter, used to get the value of the C<foo> field.

=item set_foo()

Setter, used to set the value of the C<foo> field.

=item FOO()

Constant, returns the field C<foo>'s key into the class hashref. Subclasses will
also get this function as a constant, not simply a method, that means it is
copied into the subclass namespace.

The main reason for using these constants is to help avoid spelling mistakes
and similar typos. It will not help you if you forget to prefix the '+' though.

=back

=head2 READ ONLY

    use Test2::Util::HashBase qw/-foo/;

=over 4

=item set_foo()

Throws an exception telling you the attribute is read-only. This is exported to
override any active setters for the attribute in a parent class.

=back

=head2 DEPRECATED SETTER

    use Test2::Util::HashBase qw/^foo/;

=over 4

=item set_foo()

This will set the value, but it will also warn you that the method is
deprecated.

=back

=head2 NO SETTER

    use Test2::Util::HashBase qw/<foo/;

Only gives you a reader, no C<set_foo> method is defined at all.

=head2 NO READER

    use Test2::Util::HashBase qw/>foo/;

Only gives you a write (C<set_foo>), no C<foo> method is defined at all.

=head2 CONSTANT ONLY

    use Test2::Util::HashBase qw/+foo/;

This does not create any methods for you, it just adds the C<FOO> constant.

=head1 SUBCLASSING

You can subclass an existing HashBase class.

    use base 'Another::HashBase::Class';
    use Test2::Util::HashBase qw/foo bar baz/;

The base class is added to C<@ISA> for you, and all constants from base classes
are added to subclasses automatically.

=head1 GETTING A LIST OF ATTRIBUTES FOR A CLASS

Test2::Util::HashBase provides a function for retrieving a list of attributes for an
Test2::Util::HashBase class.

=over 4

=item @list = Test2::Util::HashBase::attr_list($class)

=item @list = $class->Test2::Util::HashBase::attr_list()

Either form above will work. This will return a list of attributes defined on
the object. This list is returned in the attribute definition order, parent
class attributes are listed before subclass attributes. Duplicate attributes
will be removed before the list is returned.

B<Note:> This list is used in the C<< $class->new(\@ARRAY) >> constructor to
determine the attribute to which each value will be paired.

=back

=head1 SOURCE

The source code repository for HashBase can be found at
F<http://github.com/Test-More/HashBase/>.

=head1 MAINTAINERS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 AUTHORS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 COPYRIGHT

Copyright 2017 Chad Granum E<lt>exodist@cpan.orgE<gt>.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See F<http://dev.perl.org/licenses/>

=cut
