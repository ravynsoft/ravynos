use 5.008001; use strict; use warnings;
package TestML::Tiny;

; # original $VERSION removed by Doppelgaenger

use Carp();
use Test::More 0.88 ();

# use XXX;

sub import {
    strict->import;
    warnings->import;
}

sub new {
    my $self = bless { @_[1..$#_] }, $_[0];
    my $testml = $self->_get_testml;
    my $bridge = $self->_get_bridge;
    $self->{runtime} ||= TestML::Tiny::Runtime->new(
        bridge => $bridge,
    );
    my $compiler = TestML::Tiny::Compiler->new(
        $self->{version} ? (version => $self->{version}) : (),
    );
    $self->{function} = $compiler->compile($testml);
    return $self;
}

sub run {
    my ($self) = @_;
    my $runtime = $self->{runtime} || '';
    Carp::croak "Missing or invalid runtime object for TestML::Tiny::run()"
        unless defined($runtime) and ref($runtime) eq 'TestML::Tiny::Runtime';
    $runtime->run;
}

sub _get_testml {
    my ($self) = @_;
    my $testml = $self->{testml}
        or Carp::croak "TestML object requires a testml attribute";
    $testml = $self->_slurp($testml)
        if $testml !~ /\n/;
    return $testml;
}

sub _get_bridge {
    my ($self) = @_;
    my $bridge = $self->{bridge} || 'main';
    return $bridge if ref $bridge;
    eval "require $bridge";
    Carp::croak $@ if $@ and $@ !~ /^Can't locate /;
    return (
        defined(&{"${bridge}::new"})
            ? $bridge->new
            : bless {}, $bridge
    );
}

sub _slurp {
    open my $fh, "<:raw:encoding(UTF-8)", $_[1]
        or die "Can't open $_[1] for input";
    local $/;
    <$fh>;
}

#------------------------------------------------------------------------------

package TestML::Tiny::Runtime;

# use XXX;

sub new {
    my $self = $TestML::Tiny::Runtime::Singleton =
        bless { @_[1..$#_] }, $_[0];
};

sub run {
    Test::More::fail 'not done yet!';
    Test::More::done_testing;
}

#------------------------------------------------------------------------------
package TestML::Tiny::Compiler;

# use XXX;

my $ID = qr/\w+/;
my $SP = qr/[\ \t]/;
my $LINE = qr/.*$/m;
my $DIRECTIVE = qr/^%($ID)$SP+($LINE)/m;

sub new {
    my $self = bless { @_[1..$#_] }, $_[0];
}

sub runtime {
    $TestML::Tiny::Runtime::Singleton;
}

sub compile {
    my ($self, $testml) = @_;
    my $function = $self->{function} = TestML::Tiny::Function->new;
    $self->{testml} = $testml;
    $self->preprocess;
    my $version = $self->check_version;
    my ($code_syntax, $data_syntax) =
        @{$self}{qw(code_syntax data_syntax)};
    my $code_method = "compile_code_${code_syntax}_$version";
    Carp::croak "Don't know how to compile TestML '$code_syntax' code"
        unless $self->can($code_method);
    my $data_method = "compile_data_${data_syntax}_$version";
    Carp::croak "Don't know how to compile TestML '$data_syntax' data"
        unless $self->can($data_method);
    $function->{statements} = $self->$code_method;
    $function->{data} = $self->$data_method;
    return $function;
}

my %directives = (
    code_syntax => 'tiny',
    data_syntax => 'testml',
    data_marker => '===',
    block_marker => '===',
    point_marker => '---',
);
sub preprocess {
    my ($self) = @_;

    my $version = $self->{version} || undef;
    my $testml = $self->{testml};
    my $directives = [ $testml =~ /$DIRECTIVE/gm ];
    $testml =~ s/($DIRECTIVE)/#$1/g;
    while (@$directives) {
        my ($key, $value) = splice(@$directives, 0, 2);
        if ($key eq "TestML") {
            $self->check_not_set_and_set($key, $value, 'version');
        }
        elsif ($key eq "BlockMarker") {
            $self->check_not_set_and_set(
                'BlockMarker', $value, 'block_marker'
            );
            ($self->{block_marker} = $value) =~
                s/([\*\^\$\+\?\(\)\.])/\\$1/g;
        }
        elsif ($key eq "PointMarker") {
            $self->check_not_set_and_set(
                'PointMarker', $value, 'point_marker'
            );
            ($self->{point_marker} = $value) =~
                s/([\*\^\$\+\?\(\)\.])/\\$1/g;
        }
        elsif ($key eq "CodeSyntax") {
            die "Untested";
            $self->check_not_set_and_set(
                'CodeSyntax', $value, 'code_syntax'
            );
            $self->{code_syntax} = $value;
        }
        elsif ($key eq "DataSyntax") {
            die "Untested";
            $self->check_not_set_and_set(
                'DataSyntax', $value, 'data_syntax'
            );
            $self->{data_syntax} = $value;
        }
        else {
            Carp::croak "Unknown TestML directive: '%$key'";
        }
    }
    $self->{data_marker} = $self->{block_marker}
        if not($self->{data_marker}) and $self->{block_marker};
    for my $directive (keys %directives) {
        $self->{$directive} ||= $directives{$directive};
    }

    ($self->{code}, $self->{data}) =
        ($testml =~ /(.*?)(^$self->{data_marker}.*)/msg);
    $self->{code} ||= '';
    $self->{data} ||= '';
}

sub check_not_set_and_set {
    my ($self, $key, $value, $attr) = @_;
    if (defined $self->{$attr} and $self->{$attr} ne $value) {
        Carp::croak "Can't set TestML '$key' directive to '$value'. " .
            "Already set to '$self->{$attr}'";
    }
    $self->{$attr} = $value;
}

sub check_version {
    my ($self) = @_;
    my $version = $self->{version} || undef;
    Carp::croak "TestML syntax version not defined. Cannot continue"
        unless defined $version;
    Carp::croak "Invalid value for TestML version '$version'. Must be 0.1.0"
        unless $version eq '0.1.0';
    $version =~ s/\./_/g;
    return $version;
}

sub compile_code_tiny_0_1_0 {
    my ($self) = @_;
    my $num = 1;
    [ grep { not /(^#|^\s*$)/ } split /\n/, $self->{code} ];
}

sub compile_data_testml_0_1_0 {
    my ($self) = @_;

    my $lines = [ grep { ! /^#/ } split /\n/, $self->{data} ];

    my $blocks = [];
    my $parse = [];
    push @$lines, undef; # sentinel
    while (@$lines) {
        push @$parse, shift @$lines;
        if (!defined($lines->[0]) or
            $lines->[0] =~ /^$self->{block_marker}/
        ) {
            my $block = $self->_parse_testml_block($parse);
            push @$blocks, $block
                unless exists $block->{SKIP};
            last if exists $block->{LAST};
            $parse = []; # clear for next parse
        }
        last if !defined($lines->[0]);
    }

    my $only = [ grep { exists $_->{ONLY} } @$blocks ];

    return @$only ? $only : $blocks;
}

sub _parse_testml_block {
    my ($self, $lines) = @_;

    my ($label) = $lines->[0] =~ /^$self->{block_marker}(?:\s+(.*))?$/;
    shift @$lines until not(@$lines) or
        $lines->[0] =~ /^$self->{point_marker} +\w+/;

    my $block = $self->_parse_testml_points($lines);
    $block->{Label} = $label || '';

    return $block;
}

sub _parse_testml_points {
    my ($self, $lines) = @_;

    my $block = {};

    while (@$lines) {
        my $line = shift @$lines;
        $line =~ /^$self->{point_marker} +(\w+)/
            or die "Invalid TestML line:\n'$line'";
        my $point_name = $1;
        die "$block repeats $point_name"
            if exists $block->{$point_name};
        $block->{$point_name} = '';
        if ($line =~ /^$self->{point_marker} +(\w+): +(.*?) *$/) {
            ($block->{$1} = $2) =~ s/^ *(.*?) *$/$1/;
            shift @$lines while @$lines and
                $lines->[0] !~ /^$self->{point_marker} +(\w)/;
        }
        elsif ($line =~ /^$self->{point_marker} +(\w+)$/) {
            $point_name = $1;
            while ( @$lines ) {
                $line = shift @$lines;
                if ($line =~ /^$self->{point_marker} \w+/) {
                    unshift @$lines, $line;
                    last;
                }
                $block->{$point_name} .= "$line\n";
            }
            $block->{$point_name} =~ s/\n\s*\z/\n/;
            $block->{$point_name} =~ s/^\\//gm;
        }
        else {
            die "Invalid TestML line:\n'$line'";
        }
    }
    return $block;
}

#------------------------------------------------------------------------------
package TestML::Tiny::Function;

sub new {
    my $self = bless {
        statements => [],
        data => [],
        namespace => {},
    }, $_[0];
}

#------------------------------------------------------------------------------
package TestML::Tiny::Bridge;

sub new {
    my $self = bless { @_[1..$#_] }, $_[0];
}

#------------------------------------------------------------------------------
package TestML::Tiny::Library::Standard;

sub new {
    my $self = bless { @_[1..$#_] }, $_[0];
}

1;
