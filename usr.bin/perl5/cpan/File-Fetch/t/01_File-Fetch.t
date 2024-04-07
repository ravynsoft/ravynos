BEGIN { chdir 't' if -d 't' };

use strict;
use lib '../lib';

use Test::More 'no_plan';

use Cwd             qw[cwd];
use File::Basename  qw[basename];
use File::Path      qw[rmtree];
use Data::Dumper;

use_ok('File::Fetch');

### optionally set debugging ###
$File::Fetch::DEBUG = $File::Fetch::DEBUG   = 1 if $ARGV[0];
$IPC::Cmd::DEBUG    = $IPC::Cmd::DEBUG      = 1 if $ARGV[0];

$File::Fetch::FORCEIPV4=1;

unless( $ENV{PERL_CORE} ) {
    warn qq[

####################### NOTE ##############################

Some of these tests assume you are connected to the
internet. If you are not, or if certain protocols or hosts
are blocked and/or firewalled, these tests could fail due
to no fault of the module itself.

###########################################################

];

    sleep 3 unless $File::Fetch::DEBUG;
}

### show us the tools IPC::Cmd will use to run binary programs
if( $File::Fetch::DEBUG ) {
    ### stupid 'used only once' warnings ;(
    diag( "IPC::Run enabled: " .
            $IPC::Cmd::USE_IPC_RUN || $IPC::Cmd::USE_IPC_RUN );
    diag( "IPC::Run available: " . IPC::Cmd->can_use_ipc_run );
    diag( "IPC::Run vesion: $IPC::Run::VERSION" );
    diag( "IPC::Open3 enabled: " .
            $IPC::Cmd::USE_IPC_OPEN3 || $IPC::Cmd::USE_IPC_OPEN3 );
    diag( "IPC::Open3 available: " . IPC::Cmd->can_use_ipc_open3 );
    diag( "IPC::Open3 vesion: $IPC::Open3::VERSION" );
}

### Heuristics
my %heuristics = map { $_ => 1 } qw(http ftp rsync file git);
### _parse_uri tests
### these go on all platforms
my @map = (
    {   uri     => 'ftp://cpan.org/pub/mirror/index.txt',
        scheme  => 'ftp',
        host    => 'cpan.org',
        path    => '/pub/mirror/',
        file    => 'index.txt'
    },
    {	uri	    => 'rsync://cpan.pair.com/CPAN/MIRRORING.FROM',
        scheme	=> 'rsync',
        host	=> 'cpan.pair.com',
        path	=> '/CPAN/',
        file	=> 'MIRRORING.FROM',
    },
    {	uri	    => 'git://github.com/Perl-Toolchain-Gang/file-fetch.git',
        scheme	=> 'git',
        host	=> 'github.com',
        path	=> '/Perl-Toolchain-Gang/',
        file	=> 'file-fetch.git',
    },
    {   uri     => 'http://localhost/tmp/index.txt',
        scheme  => 'http',
        host    => 'localhost',          # host is empty only on 'file://'
        path    => '/tmp/',
        file    => 'index.txt',
    },

    ### only test host part, the rest is OS dependant
    {   uri     => 'file://localhost/tmp/index.txt',
        host    => '',                  # host should be empty on 'file://'
    },
);

### these only if we're not on win32/vms
push @map, (
    {   uri     => 'file:///usr/local/tmp/foo.txt',
        scheme  => 'file',
        host    => '',
        path    => '/usr/local/tmp/',
        file    => 'foo.txt',
    },
    {   uri     => 'file://hostname/tmp/foo.txt',
        scheme  => 'file',
        host    => 'hostname',
        path    => '/tmp/',
        file    => 'foo.txt',
    },
) if not &File::Fetch::ON_WIN and not &File::Fetch::ON_VMS;

### these only on win32
push @map, (
    {   uri     => 'file:////hostname/share/tmp/foo.txt',
        scheme  => 'file',
        host    => 'hostname',
        share   => 'share',
        path    => '/tmp/',
        file    => 'foo.txt',
    },
    {   uri     => 'file:///D:/tmp/foo.txt',
        scheme  => 'file',
        host    => '',
        vol     => 'D:',
        path    => '/tmp/',
        file    => 'foo.txt',
    },
    {   uri     => 'file:///D|/tmp/foo.txt',
        scheme  => 'file',
        host    => '',
        vol     => 'D:',
        path    => '/tmp/',
        file    => 'foo.txt',
    },
) if &File::Fetch::ON_WIN;


### sanity tests
{
    no warnings;
    like( $File::Fetch::USER_AGENT, qr/$File::Fetch::VERSION/,
                                "User agent contains version" );
    like( $File::Fetch::FROM_EMAIL, qr/@/,
                                q[Email contains '@'] );
}

### parse uri tests ###
for my $entry (@map ) {
    my $uri = $entry->{'uri'};

    my $href = File::Fetch->_parse_uri( $uri );
    ok( $href,  "Able to parse uri '$uri'" );

    for my $key ( sort keys %$entry ) {
        is( $href->{$key}, $entry->{$key},
                "   '$key' ok ($entry->{$key}) for $uri");
    }
}

### File::Fetch->new tests ###
for my $entry (@map) {
    my $ff = File::Fetch->new( uri => $entry->{uri} );

    ok( $ff,                    "Object for uri '$entry->{uri}'" );
    isa_ok( $ff, "File::Fetch", "   Object" );

    for my $acc ( keys %$entry ) {
        is( $ff->$acc(), $entry->{$acc},
                                "   Accessor '$acc' ok ($entry->{$acc})" );
    }
}

### fetch() tests ###

### file:// tests ###
{
    my $prefix = &File::Fetch::ON_UNIX ? 'file://' : 'file:///';
    my $uri = $prefix . cwd() .'/'. basename($0);

    for (qw[lwp lftp file]) {
        _fetch_uri( file => $uri, $_ );
    }
}

### Heuristics
{
  require IO::Socket::INET;
  my $sock = IO::Socket::INET->new( PeerAddr => 'mirror.bytemark.co.uk', PeerPort => 21, Timeout => 20 )
     or $heuristics{ftp} = 0;
}

### ftp:// tests ###
{   my $uri = 'ftp://mirror.bytemark.co.uk/CPAN/index.html';
    for (qw[wget curl lftp fetch ncftp]) {

        ### STUPID STUPID warnings ###
        next if $_ eq 'ncftp' and $File::Fetch::FTP_PASSIVE
                              and $File::Fetch::FTP_PASSIVE;

        _fetch_uri( ftp => $uri, $_ );
    }
}

### Heuristics
{
  require IO::Socket::INET;
  my $sock = IO::Socket::INET->new( PeerAddr => 'httpbin.org', PeerPort => 80, Timeout => 20 )
     or $heuristics{http} = 0;
}

### http:// tests ###
{   for my $uri ( 'http://httpbin.org/html',
                  'http://httpbin.org/response-headers?q=1',
                  'http://httpbin.org/response-headers?q=1&y=2',
                  #'http://www.cpan.org/index.html?q=1&y=2',
                  #'http://user:passwd@httpbin.org/basic-auth/user/passwd',
    ) {
        for (qw[lwp httptiny wget curl lftp fetch lynx httplite iosock]) {
            _fetch_uri( http => $uri, $_ );
        }
    }
}

### Heuristics
{
  require IO::Socket::INET;
  my $sock = IO::Socket::INET->new( PeerAddr => 'cpan.pair.com', PeerPort => 873, Timeout => 20 )
     or $heuristics{rsync} = 0;
}

### rsync:// tests ###
{   my $uri = 'rsync://cpan.pair.com/CPAN/MIRRORING.FROM';

    for (qw[rsync]) {
        _fetch_uri( rsync => $uri, $_ );
    }
}

### Heuristics
{
  require IO::Socket::INET;
  my $sock = IO::Socket::INET->new( PeerAddr => 'github.com', PeerPort => 9418, Timeout => 20 )
     or $heuristics{git} = 0;
}

### git:// tests ###
{   my $uri = 'https://github.com/Perl-Toolchain-Gang/file-fetch.git';

    for (qw[git]) {
        local $ENV{GIT_CONFIG_NOSYSTEM} = 1;
        local $ENV{XDG_CONFIG_HOME};
        local $ENV{HOME};
        _fetch_uri( git => $uri, $_ );
    }
}

sub _fetch_uri {
    my $type    = shift;
    my $uri     = shift;
    my $method  = shift or return;

    SKIP: {
        skip "'$method' fetching tests disabled under perl core", 4
                if $ENV{PERL_CORE};

        skip "'$type' fetching tests disabled due to heuristic failure", 4
                unless $heuristics{ $type };

        ### stupid warnings ###
        $File::Fetch::METHODS =
        $File::Fetch::METHODS = { $type => [$method] };

        ### fetch regularly
        my $ff  = File::Fetch->new( uri => $uri );

        ok( $ff,                "FF object for $uri (fetch with $method)" );

        for my $to ( 'tmp', do { \my $o } ) { SKIP: {


            my $how     = ref $to && $type ne 'git' ? 'slurp' : 'file';
            my $skip    = ref $to ? 4       : 3;

            ok( 1,              "   Fetching '$uri' in $how mode" );

            my $file = $ff->fetch( to => $to );

            skip "You do not have '$method' installed/available", $skip
                if $File::Fetch::METHOD_FAIL->{$method} &&
                   $File::Fetch::METHOD_FAIL->{$method};

            ### if the file wasn't fetched, it may be a network/firewall issue
            skip "Fetch failed; no network connectivity for '$type'?", $skip
                unless $file;

            ok( $file,          "   File ($file) fetched with $method ($uri)" );

            ### check we got some contents if we were meant to slurp
            if( ref $to && $type ne 'git' ) {
                ok( $$to,       "   Contents slurped" );
            }

            ok( $file && -s $file,
                                "   File has size" );
            is( $file && basename($file), $ff->output_file,
                                "   File has expected name" );

            rmtree $file;
        }}
    }
}








