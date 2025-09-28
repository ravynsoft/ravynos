# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
=head1 NAME

CPAN::Mirrors - Get CPAN mirror information and select a fast one

=head1 SYNOPSIS

    use CPAN::Mirrors;

    my $mirrors = CPAN::Mirrors->new( $mirrored_by_file );

    my $seen = {};

    my $best_continent = $mirrors->find_best_continents( { seen => $seen } );
    my @mirrors        = $mirrors->get_mirrors_by_continents( $best_continent );

    my $callback = sub {
        my( $m ) = @_;
        printf "%s = %s\n", $m->hostname, $m->rtt
        };
    $mirrors->get_mirrors_timings( \@mirrors, $seen, $callback, %args );

    @mirrors = sort { $a->rtt <=> $b->rtt } @mirrors;

    print "Best mirrors are ", map( { $_->rtt } @mirrors[0..3] ), "\n";

=head1 DESCRIPTION

=over

=cut

package CPAN::Mirrors;
use strict;
use vars qw($VERSION $urllist $silent);
$VERSION = "2.27";

use Carp;
use FileHandle;
use Fcntl ":flock";
use Net::Ping ();
use CPAN::Version;

=item new( LOCAL_FILE_NAME )

Create a new CPAN::Mirrors object from LOCAL_FILE_NAME. This file
should look like that in http://www.cpan.org/MIRRORED.BY .

=cut

sub new {
    my ($class, $file) = @_;
    croak "CPAN::Mirrors->new requires a filename" unless defined $file;
    croak "The file [$file] was not found" unless -e $file;

    my $self = bless {
        mirrors      => [],
        geography    => {},
    }, $class;

    $self->parse_mirrored_by( $file );

    return $self;
}

sub parse_mirrored_by {
    my ($self, $file) = @_;
    my $handle = FileHandle->new;
    $handle->open($file)
        or croak "Couldn't open $file: $!";
    flock $handle, LOCK_SH;
    $self->_parse($file,$handle);
    flock $handle, LOCK_UN;
    $handle->close;
}

=item continents()

Return a list of continents based on those defined in F<MIRRORED.BY>.

=cut

sub continents {
    my ($self) = @_;
    return sort keys %{$self->{geography} || {}};
}

=item countries( [CONTINENTS] )

Return a list of countries based on those defined in F<MIRRORED.BY>.
It only returns countries for the continents you specify (as defined
in C<continents>). If you don't specify any continents, it returns all
of the countries listed in F<MIRRORED.BY>.

=cut

sub countries {
    my ($self, @continents) = @_;
    @continents = $self->continents unless @continents;
    my @countries;
    for my $c (@continents) {
        push @countries, sort keys %{ $self->{geography}{$c} || {} };
    }
    return @countries;
}

=item mirrors( [COUNTRIES] )

Return a list of mirrors based on those defined in F<MIRRORED.BY>.
It only returns mirrors for the countries you specify (as defined
in C<countries>). If you don't specify any countries, it returns all
of the mirrors listed in F<MIRRORED.BY>.

=cut

sub mirrors {
    my ($self, @countries) = @_;
    return @{$self->{mirrors}} unless @countries;
    my %wanted = map { $_ => 1 } @countries;
    my @found;
    for my $m (@{$self->{mirrors}}) {
        push @found, $m if exists $wanted{$m->country};
    }
    return @found;
}

=item get_mirrors_by_countries( [COUNTRIES] )

A more sensible synonym for mirrors.

=cut

sub get_mirrors_by_countries { &mirrors }

=item get_mirrors_by_continents( [CONTINENTS] )

Return a list of mirrors for all of continents you specify. If you don't
specify any continents, it returns all of the mirrors.

You can specify a single continent or an array reference of continents.

=cut

sub get_mirrors_by_continents {
    my ($self, $continents ) = @_;
    $continents = [ $continents ] unless ref $continents;

    eval {
        $self->mirrors( $self->get_countries_by_continents( @$continents ) );
        };
    }

=item get_countries_by_continents( [CONTINENTS] )

A more sensible synonym for countries.

=cut

sub get_countries_by_continents { &countries }

=item default_mirror

Returns the default mirror, http://www.cpan.org/ . This mirror uses
dynamic DNS to give a close mirror.

=cut

sub default_mirror {
    CPAN::Mirrored::By->new({ http => 'http://www.cpan.org/'});
}

=item best_mirrors

C<best_mirrors> checks for the best mirrors based on the list of
continents you pass, or, without that, all continents, as defined
by C<CPAN::Mirrored::By>. It pings each mirror, up to the value of
C<how_many>. In list context, it returns up to C<how_many> mirrors.
In scalar context, it returns the single best mirror.

Arguments

    how_many      - the number of mirrors to return. Default: 1
    callback      - a callback for find_best_continents
    verbose       - true or false on all the whining and moaning. Default: false
    continents    - an array ref of the continents to check
    external_ping - if true, use external ping via Net::Ping::External. Default: false

If you don't specify the continents, C<best_mirrors> calls
C<find_best_continents> to get the list of continents to check.

If you don't have L<Net::Ping> v2.13 or later, needed for timings,
this returns the default mirror.

C<external_ping> should be set and then C<Net::Ping::External> needs
to be installed, if the local network has a transparent proxy.

=cut

sub best_mirrors {
    my ($self, %args) = @_;
    my $how_many      = $args{how_many} || 1;
    my $callback      = $args{callback};
    my $verbose       = defined $args{verbose} ? $args{verbose} : 0;
    my $continents    = $args{continents} || [];
       $continents    = [$continents] unless ref $continents;
    $args{external_ping} = 0 unless defined $args{external_ping};
    my $external_ping = $args{external_ping};

    # Old Net::Ping did not do timings at all
    my $min_version = '2.13';
    unless( CPAN::Version->vgt(Net::Ping->VERSION, $min_version) ) {
        carp sprintf "Net::Ping version is %s (< %s). Returning %s",
            Net::Ping->VERSION, $min_version, $self->default_mirror;
        return $self->default_mirror;
    }

    my $seen = {};

    if ( ! @$continents ) {
        print "Searching for the best continent ...\n" if $verbose;
        my @best_continents = $self->find_best_continents(
            seen          => $seen,
            verbose       => $verbose,
            callback      => $callback,
            external_ping => $external_ping,
            );

        # Only add enough continents to find enough mirrors
        my $count = 0;
        for my $continent ( @best_continents ) {
            push @$continents, $continent;
            $count += $self->mirrors( $self->countries($continent) );
            last if $count >= $how_many;
        }
    }

    return $self->default_mirror unless @$continents;
    print "Scanning " . join(", ", @$continents) . " ...\n" if $verbose;

    my $trial_mirrors = $self->get_n_random_mirrors_by_continents( 3 * $how_many, $continents->[0] );

    my $timings = $self->get_mirrors_timings(
        $trial_mirrors,
        $seen,
        $callback,
        %args,
    );
    return $self->default_mirror unless @$timings;

    $how_many = @$timings if $how_many > @$timings;

    return wantarray ? @{$timings}[0 .. $how_many-1] : $timings->[0];
}

=item get_n_random_mirrors_by_continents( N, [CONTINENTS] )

Returns up to N random mirrors for the specified continents. Specify the
continents as an array reference.

=cut

sub get_n_random_mirrors_by_continents {
    my( $self, $n, $continents ) = @_;
    $n ||= 3;
    $continents = [ $continents ] unless ref $continents;

    if ( $n <= 0 ) {
        return wantarray ? () : [];
    }

    my @long_list = $self->get_mirrors_by_continents( $continents );

    if ( $n eq '*' or $n > @long_list ) {
        return wantarray ? @long_list : \@long_list;
    }

    @long_list = map  {$_->[0]}
                 sort {$a->[1] <=> $b->[1]}
                 map  {[$_, rand]} @long_list;

    splice @long_list, $n; # truncate

    \@long_list;
}

=item get_mirrors_timings( MIRROR_LIST, SEEN, CALLBACK, %ARGS );

Pings the listed mirrors and returns a list of mirrors sorted in
ascending ping times.

C<MIRROR_LIST> is an anonymous array of C<CPAN::Mirrored::By> objects to
ping.

The optional argument C<SEEN> is a hash reference used to track the
mirrors you've already pinged.

The optional argument C<CALLBACK> is a subroutine reference to call
after each ping. It gets the C<CPAN::Mirrored::By> object after each
ping.

=cut

sub get_mirrors_timings {
    my( $self, $mirror_list, $seen, $callback, %args ) = @_;

    $seen = {} unless defined $seen;
    croak "The mirror list argument must be an array reference"
        unless ref $mirror_list eq ref [];
    croak "The seen argument must be a hash reference"
        unless ref $seen eq ref {};
    croak "callback must be a subroutine"
        if( defined $callback and ref $callback ne ref sub {} );

    my $timings = [];
    for my $m ( @$mirror_list ) {
        $seen->{$m->hostname} = $m;
        next unless eval{ $m->http };

        if( $self->_try_a_ping( $seen, $m, ) ) {
            my $ping = $m->ping(%args);
            next unless defined $ping;
            # printf "m %s ping %s\n", $m, $ping;
            push @$timings, $m;
            $callback->( $m ) if $callback;
        }
        else {
            push @$timings, $seen->{$m->hostname}
                if defined $seen->{$m->hostname}->rtt;
        }
    }

    my @best = sort {
           if( defined $a->rtt and defined $b->rtt )     {
            $a->rtt <=> $b->rtt
            }
        elsif( defined $a->rtt and ! defined $b->rtt )   {
            return -1;
            }
        elsif( ! defined $a->rtt and defined $b->rtt )   {
            return 1;
            }
        elsif( ! defined $a->rtt and ! defined $b->rtt ) {
            return 0;
            }

        } @$timings;

    return wantarray ? @best : \@best;
}

=item find_best_continents( HASH_REF );

C<find_best_continents> goes through each continent and pings C<N>
random mirrors on that continent. It then orders the continents by
ascending median ping time. In list context, it returns the ordered list
of continent. In scalar context, it returns the same list as an
anonymous array.

Arguments:

    n        - the number of hosts to ping for each continent. Default: 3
    seen     - a hashref of cached hostname ping times
    verbose  - true or false for noisy or quiet. Default: false
    callback - a subroutine to run after each ping.
    ping_cache_limit - how long, in seconds, to reuse previous ping times.
        Default: 1 day

The C<seen> hash has hostnames as keys and anonymous arrays as values.
The anonymous array is a triplet of a C<CPAN::Mirrored::By> object, a
ping time, and the epoch time for the measurement.

The callback subroutine gets the C<CPAN::Mirrored::By> object, the ping
time, and measurement time (the same things in the C<seen> hashref) as
arguments. C<find_best_continents> doesn't care what the callback does
and ignores the return value.

With a low value for C<N>, a single mirror might skew the results enough
to choose a worse continent. If you have that problem, try a larger
value.

=cut

sub find_best_continents {
    my ($self, %args) = @_;

    $args{n}     ||= 3;
    $args{verbose} = 0 unless defined $args{verbose};
    $args{seen}    = {} unless defined $args{seen};
    croak "The seen argument must be a hash reference"
        unless ref $args{seen} eq ref {};
    $args{ping_cache_limit} = 24 * 60 * 60
        unless defined $args{ping_cache_limit};
    croak "callback must be a subroutine"
        if( defined $args{callback} and ref $args{callback} ne ref sub {} );

    my %medians;
    CONT: for my $c ( $self->continents ) {
        my @mirrors = $self->mirrors( $self->countries($c) );
        printf "Testing %s (%d mirrors)\n", $c, scalar @mirrors
            if $args{verbose};

        next CONT unless @mirrors;
        my $n = (@mirrors < $args{n}) ? @mirrors : $args{n};

        my @tests;
        my $tries = 0;
        RANDOM: while ( @mirrors && @tests < $n && $tries++ < 15 ) {
            my $m = splice( @mirrors, int(rand(@mirrors)), 1 );
            if( $self->_try_a_ping(
                    $args{seen}, $m, $args{ping_cache_limit}
                )) {
                $self->get_mirrors_timings(
                    [ $m ],
                    $args{seen},
                    $args{callback},
                    %args,
                );
                next RANDOM unless defined $args{seen}{$m->hostname}->rtt;
            }
            printf "(%s -> %0.2f ms)",
                $m->hostname,
                join ' ', 1000 * $args{seen}{$m->hostname}->rtt
                    if $args{verbose};

            push @tests, $args{seen}{$m->hostname}->rtt;
        }

        my $median = $self->_get_median_ping_time( \@tests, $args{verbose} );
        $medians{$c} = $median if defined $median;
    }

    my @best_cont = sort { $medians{$a} <=> $medians{$b} } keys %medians;

    if ( $args{verbose} ) {
        print "Median result by continent:\n";
        if ( @best_cont ) {
            for my $c ( @best_cont ) {
                printf( "  %7.2f ms  %s\n", $medians{$c}*1000, $c );
            }
        } else {
            print "  **** No results found ****\n"
        }
    }

    return wantarray ? @best_cont : $best_cont[0];
}

# retry if
sub _try_a_ping {
    my ($self, $seen, $mirror, $ping_cache_limit ) = @_;

    ( ! exists $seen->{$mirror->hostname}
        or
    ! defined $seen->{$mirror->hostname}->rtt
      or
    ! defined $ping_cache_limit
      or
      time - $seen->{$mirror->hostname}->ping_time
        > $ping_cache_limit
    )
}

sub _get_median_ping_time {
    my ($self, $tests, $verbose ) = @_;

    my @sorted = sort { $a <=> $b } @$tests;

    my $median = do {
           if ( @sorted == 0 ) { undef }
        elsif ( @sorted == 1 ) { $sorted[0] }
        elsif ( @sorted % 2 )  { $sorted[ int(@sorted / 2) ] }
        else {
            my $mid_high = int(@sorted/2);
            ($sorted[$mid_high-1] + $sorted[$mid_high])/2;
        }
    };

    if ($verbose){
        if ($median) {
            printf " => median time: %.2f ms\n", $median * 1000
        } else {
            printf " => **** no median time ****\n";
        }
    }

    return $median;
}

# Adapted from Parse::CPAN::MirroredBy by Adam Kennedy
sub _parse {
    my ($self, $file, $handle) = @_;
    my $output = $self->{mirrors};
    my $geo    = $self->{geography};

    local $/ = "\012";
    my $line = 0;
    my $mirror = undef;
    while ( 1 ) {
        # Next line
        my $string = <$handle>;
        last if ! defined $string;
        $line = $line + 1;

        # Remove the useless lines
        chomp( $string );
        next if $string =~ /^\s*$/;
        next if $string =~ /^\s*#/;

        # Hostname or property?
        if ( $string =~ /^\s/ ) {
            # Property
            unless ( $string =~ /^\s+(\w+)\s+=\s+\"(.*)\"$/ ) {
                croak("Invalid property on line $line");
            }
            my ($prop, $value) = ($1,$2);
            $mirror ||= {};
            if ( $prop eq 'dst_location' ) {
                my (@location,$continent,$country);
                @location = (split /\s*,\s*/, $value)
                    and ($continent, $country) = @location[-1,-2];
                $continent =~ s/\s\(.*//;
                $continent =~ s/\W+$//; # if Jarkko doesn't know latitude/longitude
                $geo->{$continent}{$country} = 1 if $continent && $country;
                $mirror->{continent} = $continent || "unknown";
                $mirror->{country} = $country || "unknown";
            }
            elsif ( $prop eq 'dst_http' ) {
                $mirror->{http} = $value;
            }
            elsif ( $prop eq 'dst_ftp' ) {
                $mirror->{ftp} = $value;
            }
            elsif ( $prop eq 'dst_rsync' ) {
                $mirror->{rsync} = $value;
            }
            else {
                $prop =~ s/^dst_//;
                $mirror->{$prop} = $value;
            }
        } else {
            # Hostname
            unless ( $string =~ /^([\w\.-]+)\:\s*$/ ) {
                croak("Invalid host name on line $line");
            }
            my $current = $mirror;
            $mirror     = { hostname => "$1" };
            if ( $current ) {
                push @$output, CPAN::Mirrored::By->new($current);
            }
        }
    }
    if ( $mirror ) {
        push @$output, CPAN::Mirrored::By->new($mirror);
    }

    return;
}

#--------------------------------------------------------------------------#

package CPAN::Mirrored::By;
use strict;
use Net::Ping   ();

sub new {
    my($self,$arg) = @_;
    $arg ||= {};
    bless $arg, $self;
}
sub hostname  { shift->{hostname}    }
sub continent { shift->{continent}   }
sub country   { shift->{country}     }
sub http      { shift->{http}  || '' }
sub ftp       { shift->{ftp}   || '' }
sub rsync     { shift->{rsync} || '' }
sub rtt       { shift->{rtt}         }
sub ping_time { shift->{ping_time}   }

sub url {
    my $self = shift;
    return $self->{http} || $self->{ftp};
}

sub ping {
    my($self, %args) = @_;

    my $external_ping = $args{external_ping};
    if ($external_ping) {
        eval { require Net::Ping::External }
            or die "Net::Ping::External required to use external ping command";
    }
    my $ping = Net::Ping->new(
        $external_ping ? 'external' : $^O eq 'VMS' ? 'icmp' : 'tcp',
        1
    );
    my ($proto) = $self->url =~ m{^([^:]+)};
    my $port = $proto eq 'http' ? 80 : 21;
    return unless $port;

    if ( $ping->can('port_number') ) {
        $ping->port_number($port);
    }
    else {
        $ping->{'port_num'} = $port;
    }

    $ping->hires(1) if $ping->can('hires');
    my ($alive,$rtt) = eval { $ping->ping($self->hostname); };
    my $verbose = $args{verbose};
    if ($verbose && !$alive) {
        printf "(host %s not alive)", $self->hostname;
    }

    $self->{rtt} = $alive ? $rtt : undef;
    $self->{ping_time} = time;

    $self->rtt;
}


1;

=back

=head1 AUTHOR

Andreas Koenig C<< <andk@cpan.org> >>, David Golden C<< <dagolden@cpan.org> >>,
brian d foy C<< <bdfoy@cpan.org> >>

=head1 LICENSE

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See L<http://www.perl.com/perl/misc/Artistic.html>

=cut
